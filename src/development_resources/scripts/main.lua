----------------
--- System
--[RENDERING]
--    drawSprite(string: name, number: x, number: y) => nil
--      draw a sprite on the screen at position x,y (from the top left)
--      the sprite is the PGN located at resources/textures/name.png
--      sprite anchors to its top left
--
--    setPen(number: r, number: g, number: b, number: a) => nil
--      set the current pen color in RGBA; values are from 0-1
--      the pen color is used for any drawing primitives (e.g., drawRect)
--      
--    drawRect(number: x, number: y, number: width, number: height) => nil
--      draw a topleft-anchored rectangle at screen position x,y (from the top left)
--
--[INPUTS]
--    getActionPressed(string: action) => bool
--      return trues on the frame when an action is first down
--      action can be one of: up, down, left, right, confirm, cancel, start,
--
--    getActionHeld(string: action) => bool
--      returns true every frame that an action is down
--      action can be one of: up, down, left, right, confirm, cancel, start
--
--[AUDIO]
--    playSFX(string: name) => nil
--      plays the WAV file located at /resources/audio/name.wav
--
--    playMusic(string: name, number: duration) => nil
--      plays, on loop, the WAV file located at /resources/audio/name.wav
--      duration specifies a fade-in time
--      if another music file is playing, the two will cross-fade over duration
--
--    stopMusic(number: duration) => nil
--      stops any playing music, fading over the requested duration
--
----------------
--- World
--    setPaused(bool: paused) => nil
--      set whether the world simulation is paused
--      while the world simulation is running, update(dt) will be called from the engine
--      while the world simulation is paused, update_paused(dt) will be called


-- testing require to load other modules
local player = require "player"

-- global vars

-- callback triggered after the engine initializes
function init()
  System.playSFX("test1")
  World.loadWorld("map")
  World.loadEvents("ZeldaLike")
  World.startDialogue({"Oh... Who am I?", "Oh no! I'm am have the amnesia!"})
end

-- callback triggered at the physics update rate while the game world is not paused
function update(dt)
  player.move(dt)
end

-- callback triggered at the physics update rate while the game world is not paused
function update_paused(dt)

end

-- callback that is (hopefully) triggered at the target framerate
function draw()

end
