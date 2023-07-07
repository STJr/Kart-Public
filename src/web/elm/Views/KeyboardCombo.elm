module Views.KeyboardCombo exposing (init, initKey, toHtml, withKey)

import Html exposing (Html, div, span, text)
import Html.Attributes exposing (class)


type Key
    = Key { label : String }


type KeyboardCombo
    = KeyboardCombo { keys : List Key }



-- Public


init : KeyboardCombo
init =
    KeyboardCombo { keys = [] }


initKey : String -> Key
initKey label =
    Key { label = label }


withKey : Key -> KeyboardCombo -> KeyboardCombo
withKey key (KeyboardCombo kcombo) =
    KeyboardCombo { kcombo | keys = kcombo.keys ++ [ key ] }


keyToHtml : Key -> Html msg
keyToHtml (Key key) =
    span [ class "border rounded px-2 py-1 shadow" ] [ text key.label ]


toHtml : KeyboardCombo -> Html msg
toHtml (KeyboardCombo kcombo) =
    kcombo.keys
        |> List.map keyToHtml
        |> List.intersperse (span [] [ text "+" ])
        |> div []
