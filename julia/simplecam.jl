module Cam

import GLFW
import GR

type FreeCamera
    camera::GR.Camera
    window::GLFW.Window
    deltaTrans::Vector{Float32}
    deltaAngles::Vector{Float32}
    lastTime::Float64

    Camera(renderer::GR.Renderer, window::GLFW.Window, deltaTrans::Vector{Float32}, deltaAngles::Vector{Float32}) = new(renderer.camera, window, deltaTrans, deltaAngles, time())
end

function update(cam::FreeCamera, trans::Vector{Float32}, angles::Vector{Float32}, timeNow::Float64)
    x = m[1, 1:3]'
    y = m[2, 1:3]'
    z = m[3, 1:3]'

    translation = x*trans[1] + y*trans[2] + z*trans[3]

    rot =
end

function process_input(cam::FreeCamera)
    trans = Array(Float32, 3)
    angles = Array(Float32, 3)
    timeNow = time()
    deltaTime = timeNow - cam.lastTime

    trans[1] = (GLFW.GetKey(cam.window, GLFW.KEY_D) - GLFW.GetKey(cam.window, GLFW.KEY_A)) * camera.deltaTrans[1] * deltaTime
    trans[2] = (GLFW.GetKey(cam.window, GLFW.KEY_G) - GLFW.GetKey(cam.window, GLFW.KEY_T)) * camera.deltaTrans[2] * deltaTime
    trans[3] = (GLFW.GetKey(cam.window, GLFW.KEY_S) - GLFW.GetKey(cam.window, GLFW.KEY_W)) * camera.deltaTrans[3] * deltaTime

    angles[1] = (GLFW.GetKey(cam.window, GLFW.KEY_F) - GLFW.GetKey(cam.window, GLFW.KEY_R)) * camera.deltaAngles[1] * deltaTime
    angles[2] = (GLFW.GetKey(cam.window, GLFW.KEY_Q) - GLFW.GetKey(cam.window, GLFW.KEY_E)) * camera.deltaAngles[2] * deltaTime
    angles[3] = (GLFW.GetKey(cam.window, GLFW.KEY_Z) - GLFW.GetKey(cam.window, GLFW.KEY_C)) * camera.deltaAngles[3] * deltaTime

    update(cam, trans, angles, timeNow)
end

end

