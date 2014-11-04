module Cam

import GLFW
import GR

type FreeCamera
    camera::GR.Camera
    window::GLFW.Window
    lastTime::Float64
end

function process_input(cam::FreeCamera)

end
	
end