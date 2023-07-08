module Msg exposing (Msg(..))

import Bytes exposing (Bytes)
import File exposing (File)


type Msg
    = StartGame
    | GotGameOutput String
    | GotStatusMessage String
    | RequestFullScreen
    | ShowHelp
    | HideHelp
    | ShowAddonPicker
    | HideAddonPicker
    | AddWadFile String
    | AddonLoaded File
    | ClickedAddAddon
    | FileBytesDecoded String Bytes
