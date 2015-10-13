# tests, moved from FTFont.jl
import FTFont
import Math2D

import Images

function testdraw(canvas, pos, bmp, box)
    dst = Math2D.Vec2{Int}(pos.x, pos.y)
    dstMax = dst + size(box)
    canvas[dst.x:dstMax.x, dst.y:dstMax.y] += bmp[box.min.x:box.max.x, box.min.y:box.max.y]
end

global font
function fttest()
    FTFont.init()

    global font = FTFont.loadfont("data/Roboto-Regular.ttf"; chars = ['\u0000':'\u00ff', 'А':'Я', 'а':'я'])
    Images.imwrite(Images.grayim(font.bitmap), "font.png")

    FTFont.done()

    canvas = zeros(UInt8, 1024, 256)
    cursor = FTFont.TextCursor(font.size.x, font.lineDistance)
    drawFunc = (pos, bmp, box)->testdraw(canvas, pos, bmp, box)
    FTFont.drawtext(drawFunc, font, cursor, "The quick brown fox jumps over the lazy dog!")

    FTFont.setpos(cursor, font.size.x, 2font.lineDistance)
    FTFont.drawtext(drawFunc, font, cursor, "Aveline F")
    FTFont.drawtext(drawFunc, font, cursor, ".") # if the font supports kerning, the '.' should be moved closer to the 'F' at the end of the previous drawtext()
    FTFont.drawtext(drawFunc, font, cursor, "\u1000")

    FTFont.setpos(cursor, font.size.x, 3font.lineDistance)
    s = "Яваш щета фира?"
    box = FTFont.textbox(font, cursor, s)
    FTFont.drawtext(drawFunc, font, cursor, s)
    box.min -= one(box.min)
    box.max += one(box.max)
    canvas[box.min.x:box.max.x, box.min.y] = 255
    canvas[box.min.x:box.max.x, box.max.y] = 255
    canvas[box.min.x, box.min.y:box.max.y] = 255
    canvas[box.max.x, box.min.y:box.max.y] = 255

    Images.imwrite(Images.grayim(canvas), "fox.png")
end
