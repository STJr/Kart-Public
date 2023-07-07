module Views.Button exposing (RequiredButtonParams, init, toHtml)

import Html exposing (Html, button, text)
import Html.Attributes exposing (class)
import Html.Events exposing (onClick)


type Button msg
    = Button
        { text : String
        , onClick : msg
        }


type alias RequiredButtonParams msg =
    { text : String
    , onClick : msg
    }



-- Public


init : RequiredButtonParams msg -> Button msg
init params =
    Button { text = params.text, onClick = params.onClick }


toHtml : Button msg -> Html msg
toHtml (Button button_) =
    button
        [ onClick button_.onClick
        , class "transition duration-150 border-solid px-5 py-2 rounded bg-blue-600 text-white shadow-md hover:bg-blue-400 active:shadow-none active:scale-95"
        ]
        [ text button_.text ]
