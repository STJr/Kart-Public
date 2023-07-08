module Views.Help exposing (view)

import Html exposing (Html, h2, li, section, span, text, ul)
import Html.Attributes exposing (class)
import Msg exposing (Msg(..))
import Views.KeyboardCombo
import Views.Modal


view : Bool -> Html Msg
view isOpen =
    Views.Modal.init
        { header = "Welcome to SRB2Kart WASM!"
        , content =
            [ section []
                [ h2 [ class "text-lg mb-2" ] [ text "Controls" ]
                , controlsView
                ]
            ]
        , isOpen = isOpen
        , onClose = HideHelp
        }
        |> Views.Modal.toHtml



-- INTERNAL


controlsView : Html msg
controlsView =
    let
        control description combo =
            li [ class "flex gap-2" ]
                [ Views.KeyboardCombo.toHtml combo
                , span [] [ text " : " ]
                , span [] [ text description ]
                ]
    in
    ul [ class "list-none space-y-3" ]
        [ Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "←")
            |> control "Steer Left"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "→")
            |> control "Steer Right"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "↑")
            |> control "Up"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "↓")
            |> control "Down"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "A")
            |> control "Accelerate"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "S")
            |> control "Break"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "Space")
            |> control "Use Item"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "Enter")
            |> control "Confirm"
        , Views.KeyboardCombo.init
            |> Views.KeyboardCombo.withKey (Views.KeyboardCombo.initKey "Esc")
            |> control "Return / Open Menu / Pause"
        ]
