module Views.Help exposing (view)

import Html exposing (Html, article, h1, h2, header, li, main_, section, span, text, ul)
import Html.Attributes exposing (class)
import Msg exposing (Msg(..))
import Views.Button
import Views.KeyboardCombo


view : Html Msg
view =
    article [ class "absolute min-w-fit h-1/2 overflow-auto flex items-center flex-col bg-black text-white rounded opacity-95" ]
        [ header [ class "flex" ]
            [ h1
                [ class "m-5 text-xl font-bold" ]
                [ text "Welcome to SRB2Kart WASM!" ]
            , Views.Button.init { text = "X", onClick = HideHelp } |> Views.Button.toHtml
            ]
        , main_ []
            [ section []
                [ h2 [ class "text-lg mb-2" ] [ text "Controls" ]
                , controlsView
                ]
            ]
        ]



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
