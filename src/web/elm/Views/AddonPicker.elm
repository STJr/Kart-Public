module Views.AddonPicker exposing (..)

import Html exposing (Html, h3, li, section, text, ul)
import Msg exposing (Msg(..))
import Views.Modal


view : Bool -> List String -> Html Msg
view isOpen addons =
    Views.Modal.init
        { header = "Addon Manager"
        , content =
            [ section []
                [ h3 [] [ text "Installed Addons" ]
                , ul [] <| List.map (\addon -> li [] [ text addon ]) addons
                ]
            ]
        , isOpen = isOpen
        , onClose = HideAddonPicker
        }
        |> Views.Modal.toHtml
