--[[ 
Script for Angry Nerda
Hadles the available types of shots

For each shot the info needed is:
input key string
input key
input ascii code
max shots
density
shot image
size
shot fixed image
size
character image
size x
size y

By Dario Urdapilleta
--]]
shots = {
			{"Z", "90", "100", "2", "NONE", "NONE", "Media\\Cannonball.png", "36", "NONE", "NONE", "NONE", "100", "3"}, 
			{"X", "88", "2", "2", "Media\\CannonBallFuse.png", "52", "Media\\Cannonball.png", "36", "Media\\BombNerd.png", "393", "596", "500", "1"}, 
			{"C", "67", "2", "2", "Media\\MultiBomb.png", "36", "Media\\MultiBombShadow.png", "36", "Media\\MultiNerd.png", "412", "552", "500", "2"}, 
			{"NONE", "NONE", "0", "8", "Media\\SmallBall.png", "13", "NONE", "13", "NONE", "NONE", "NONE", "0", "3"}
		}
--[[ Returns to AngryNerds the amount of shots --]]
function getShotsSize()
	return table.getn(shots)
end
--[[  Returns a specific shot's information --]]
function getShotInfo(n)
	return shots[n][13], shots[n][12], shots[n][11], shots[n][10], shots[n][9], shots[n][8], shots[n][7], shots[n][6], shots[n][5], shots[n][4], shots[n][3], shots[n][2], shots[n][1]
end
--[[ Does the actions of the shots depending on their actionID --]]
function activateAbility(actionID)
	if actionID == 1 then
		i = ApplyForceWithinRange(5)
		i = ShowExplosion()
		i = DestroyCurrentShot()
	elseif actionID == 2 then
		for ballNumber = 0, 5 do
			i = MakeNewBall(ballNumber, 3, 2)
			i = DestroyCurrentShot()
		end
	elseif actionID == 3 then
		i = ShowExplosion()
		i = DestroyCurrentShot()
	end
end