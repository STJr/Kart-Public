module Views.Modal exposing (init, toHtml)

import Html exposing (Html, article, h1, h2, header, main_, section, text)
import Html.Attributes exposing (class)
import Msg exposing (Msg(..))
import Views.Button


type Modal msg
    = Modal
        { header : String
        , content : List (Html msg)
        , isOpen : Bool
        , onClose : msg
        }


type alias RequiredParams msg =
    { header : String
    , content : List (Html msg)
    , isOpen : Bool
    , onClose : msg
    }


init : RequiredParams msg -> Modal msg
init { header, content, isOpen, onClose } =
    Modal
        { header = header
        , content = content
        , isOpen = isOpen
        , onClose = onClose
        }


toHtml : Modal msg -> Html msg
toHtml (Modal modal) =
    article
        [ class "absolute min-w-fit h-1/2 overflow-auto flex items-center flex-col bg-black text-white rounded opacity-95"
        , class
            (if modal.isOpen then
                "block"

             else
                "hidden"
            )
        ]
        [ header [ class "flex" ]
            [ h1
                [ class "m-5 text-xl font-bold" ]
                [ text modal.header ]
            , Views.Button.init { text = "X", onClick = modal.onClose } |> Views.Button.toHtml
            ]
        , main_ [] modal.content
        ]
