module Main exposing (..)

import Browser
import Html exposing (Html, button, div, text)
import Html.Events exposing (onClick)



-- MAIN


main =
  Browser.document { init = init, update = update, view = document, subscriptions = \_ -> Sub.none }



-- MODEL

type alias Model = Int

init : () -> (Model, Cmd Msg)
init flags =
  (0, Cmd.none)


-- UPDATE

type Msg = Increment | Decrement

update : Msg -> Model -> (Model, Cmd Msg)
update msg model =
  case msg of
    Increment ->
      (model + 1, Cmd.none)

    Decrement ->
      (model - 1, Cmd.none)


-- VIEW

document : Model -> Browser.Document Msg
document model =
    { body =  view model
    , title = "SRB2Kart v1.6"
    }

view : Model -> List (Html Msg)
view model =
  [ div []
    [ button [ onClick Decrement ] [ text "-" ]
    , div [] [ text (String.fromInt model) ]
    , button [ onClick Increment ] [ text "+" ]
    ]
  ]
