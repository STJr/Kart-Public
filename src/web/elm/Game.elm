module Game exposing (Game, getAddons, init, setAddons)


type Game
    = Game
        { addons : List String
        }


init : Game
init =
    Game { addons = [] }


setAddons : List String -> Game -> Game
setAddons addons (Game game) =
    Game { game | addons = addons }


getAddons : Game -> List String
getAddons (Game game) =
    game.addons
