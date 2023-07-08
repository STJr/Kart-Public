port module Main exposing (..)

import Addon
import Base64
import Browser
import Bytes.Encode
import CommandHandler
import Commands exposing (Command(..))
import Commands.ListWads
import File
import File.Select as Select
import Game exposing (Game)
import Html exposing (Html, article, button, canvas, div, h1, h2, header, li, main_, p, progress, section, span, text, ul)
import Html.Attributes exposing (class, classList, height, hidden, id, max, value, width)
import Html.Events exposing (onClick)
import Html.Lazy exposing (lazy)
import Msg exposing (Msg(..))
import Status exposing (Status(..))
import Task
import Views.AddonPicker
import Views.Button
import Views.Help


port startGame : () -> Cmd msg


port requestFullScreen : () -> Cmd msg


port listWads : () -> Cmd msg


port addFile : () -> Cmd msg


port gameOutput : (String -> msg) -> Sub msg


port statusMessage : (String -> msg) -> Sub msg



-- MAIN


main : Program () Model Msg
main =
    Browser.document { init = init, update = update, view = document, subscriptions = subscriptions }



-- MODEL


type alias Model =
    { outputLines : List String
    , emStatus : Status
    , helpShown : Bool
    , addonPickerShown : Bool
    , game : Game
    , commandState : Maybe Command
    }


init : () -> ( Model, Cmd Msg )
init _ =
    ( { outputLines = []
      , emStatus = Status.NotStarted
      , helpShown = False
      , addonPickerShown = False
      , game = Game.init
      , commandState = Maybe.Nothing
      }
    , Cmd.none
    )


subscriptions : Model -> Sub Msg
subscriptions _ =
    Sub.batch
        [ gameOutput GotGameOutput
        , statusMessage GotStatusMessage
        ]



-- UPDATE


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        StartGame ->
            ( model, startGame () )

        GotGameOutput line ->
            let
                handleLineResult =
                    Commands.handleLine line model.commandState model.game
            in
            case handleLineResult of
                CommandHandler.Finished game ->
                    ( { model | commandState = Maybe.Nothing, game = game }, Cmd.none )

                CommandHandler.Pending newState ->
                    ( { model | commandState = Just (Commands.ListWads newState) }, Cmd.none )

                CommandHandler.Error ->
                    ( model, Cmd.none )

                CommandHandler.Nothing ->
                    ( model, Cmd.none )

        GotStatusMessage line ->
            case Status.parse line of
                Ok Status.Nothing ->
                    ( model, Cmd.none )

                Ok status ->
                    ( { model | emStatus = status }, Cmd.none )

                _ ->
                    ( { model | emStatus = Status.Error }, Cmd.none )

        RequestFullScreen ->
            ( model, requestFullScreen () )

        ShowHelp ->
            ( { model | helpShown = True }, Cmd.none )

        HideHelp ->
            ( { model | helpShown = False }, Cmd.none )

        HideAddonPicker ->
            ( { model | addonPickerShown = False }, Cmd.none )

        ShowAddonPicker ->
            ( { model
                | commandState = Just (Commands.ListWads Commands.ListWads.init)
                , addonPickerShown = True
              }
            , listWads ()
            )

        ClickedAddAddon ->
            ( model, addFile () )



-- VIEW


document : Model -> Browser.Document Msg
document model =
    { body = view model
    , title = "SRB2Kart v1.6"
    }


viewCanvas : Model -> Html Msg
viewCanvas model =
    lazy
        (canvas
            [ id "canvas"
            , class "border-double border-8 border-blue-100 rounded-md shadow-md"
            , if model.emStatus == Running then
                class "block"

              else
                class "hidden"
            , width 500
            , height 500
            ]
        )
        []


viewConsole : List String -> Html Msg
viewConsole lines =
    div [ class "hidden" ] [ ul [] <| List.map (\line -> li [] [ text line ]) lines ]


viewControls : Status -> Html Msg
viewControls status =
    case status of
        Status.NotStarted ->
            div [ class "flex flex-col items-center" ]
                [ Views.Button.init { text = "Start", onClick = StartGame } |> Views.Button.toHtml
                , p [] [ span [] [ text "WARNING: This will download approximately 500MB of data" ] ]
                ]

        Status.Running ->
            div [ class "flex gap-2" ]
                [ Views.Button.init { text = "Fullscreen", onClick = RequestFullScreen } |> Views.Button.toHtml
                , Views.Button.init { text = "Help", onClick = ShowHelp } |> Views.Button.toHtml
                , Views.Button.init { text = "Addon Manager", onClick = ShowAddonPicker } |> Views.Button.toHtml
                , Views.Button.init { text = "Add Addon", onClick = ClickedAddAddon } |> Views.Button.toHtml
                ]

        _ ->
            text ""


viewStatus : Status -> Html Msg
viewStatus status =
    case status of
        NotStarted ->
            text ""

        Running ->
            text ""

        _ ->
            span [ class "text-red-700" ] [ text <| Status.toString status ]


view : Model -> List (Html Msg)
view model =
    [ Html.main_ [ class "w-screen h-screen flex items-center justify-center flex-col gap-2" ]
        [ viewCanvas model
        , viewConsole model.outputLines
        , viewControls model.emStatus
        , viewStatus model.emStatus
        , Views.Help.view model.helpShown
        , Views.AddonPicker.view model.addonPickerShown <| Game.getAddons model.game
        ]
    ]
