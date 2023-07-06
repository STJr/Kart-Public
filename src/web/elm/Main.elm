port module Main exposing (..)

import Browser
import Html exposing (Html, button, canvas, div, progress, text)
import Html.Attributes exposing (height, hidden, id, max, value, width)
import Html.Events exposing (onClick)


port startGame : () -> Cmd msg



-- MAIN


main =
    Browser.document { init = init, update = update, view = document, subscriptions = \_ -> Sub.none }



-- MODEL


type alias Model =
    Int


init : () -> ( Model, Cmd Msg )
init flags =
    ( 0, Cmd.none )



-- UPDATE


type Msg
    = Increment
    | Decrement
    | StartGame


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Increment ->
            ( model + 1, Cmd.none )

        Decrement ->
            ( model - 1, Cmd.none )

        StartGame ->
            ( model, startGame () )



-- VIEW


document : Model -> Browser.Document Msg
document model =
    { body = view model
    , title = "SRB2Kart v1.6"
    }


view : Model -> List (Html Msg)
view model =
    [ div []
        [ button [ onClick StartGame ] [ text "Start" ]
        ]
    , div [ id "spinner" ] []
    , div [ id "status" ] []
    , progress [ value "0", Html.Attributes.max "100", id "progress", hidden True ] []
    , div [ id "spinner" ] []
    , canvas [ id "canvas", width 500, height 500 ] []
    ]
