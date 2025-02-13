module SimpleCam

import GLFW
import GRU

const Math3D = GRU.Math3D

type FreeCamera
    camera::GRU.Camera
    window::GLFW.Window
    deltaTrans::Vector{Float32}
    deltaAngles::Vector{Float32}
    lastTime::Float64

    FreeCamera(renderer::GRU.Renderer, window::GLFW.Window, deltaTrans::Vector{Float32}, deltaAngles::Vector{Float32}) = new(renderer.camera, window, deltaTrans, deltaAngles, time())
end

function update(cam::FreeCamera, trans::Vector{Float32}, angles::Vector{Float32}, timeNow::Float64)
    m = eye(Float32, 4, 4)
    Math3D.rotxyz(m, angles...)
    Math3D.trans(m, trans)
    GRU.settransform(cam.camera, GRU.gettransform(cam.camera)*m)
    cam.lastTime = timeNow
end

function process_input(cam::FreeCamera)
    trans = Array(Float32, 3)
    angles = Array(Float32, 3)
    timeNow = time()
    deltaTime = timeNow - cam.lastTime

    trans[1] = (GLFW.GetKey(cam.window, GLFW.KEY_D) - GLFW.GetKey(cam.window, GLFW.KEY_A)) * cam.deltaTrans[1] * deltaTime
    trans[2] = (GLFW.GetKey(cam.window, GLFW.KEY_F) - GLFW.GetKey(cam.window, GLFW.KEY_R)) * cam.deltaTrans[2] * deltaTime
    trans[3] = (GLFW.GetKey(cam.window, GLFW.KEY_W) - GLFW.GetKey(cam.window, GLFW.KEY_S)) * cam.deltaTrans[3] * deltaTime

    angles[1] = (GLFW.GetKey(cam.window, GLFW.KEY_G) - GLFW.GetKey(cam.window, GLFW.KEY_T)) * cam.deltaAngles[1] * deltaTime
    angles[2] = (GLFW.GetKey(cam.window, GLFW.KEY_E) - GLFW.GetKey(cam.window, GLFW.KEY_Q)) * cam.deltaAngles[2] * deltaTime
    angles[3] = (GLFW.GetKey(cam.window, GLFW.KEY_Z) - GLFW.GetKey(cam.window, GLFW.KEY_C)) * cam.deltaAngles[3] * deltaTime

    update(cam, trans, angles, timeNow)
end

end
