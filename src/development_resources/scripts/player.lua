function move(dt) 
  local dx = 0
  local dy = 0
  if System.getActionHeld("right") then dx = dx + 1 end
  if System.getActionHeld("left") then dx = dx - 1 end
  if System.getActionHeld("up") then dy = dy - 1 end
  if System.getActionHeld("down") then dy = dy + 1 end
  local speed = 60
  World.movePlayer(dx * speed * dt, dy * speed * dt)
end

return { move = move } -- local symbols to be exported via require