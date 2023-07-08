module Msg exposing (Msg(..))


type Msg
    = StartGame
    | GotGameOutput String
    | GotStatusMessage String
    | RequestFullScreen
    | ShowHelp
    | HideHelp
    | ShowAddonPicker
    | HideAddonPicker
    | ClickedAddAddon
