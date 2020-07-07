#!/usr/bin/ruby --disable=all

require('optionparser')

# generates a pyramid

def gen(sides, r, b, h)
  step = 360.0 / sides

 [[0, b + h, 0]] +
    sides.times.map do |i|
      theta = Math::PI * i * step / 180.0

      [r * Math.cos(theta), b, r * Math.sin(theta)].map { |n| n.round(4) }
    end
end

# s sides
# r radius
# b base lvl
# h height
defaults = [12, 0.6, -1.0, 2.3]
params = ARGV.getopts('s:r:b:h:')
  .values
  .each_with_index
  .map { |e, i| e ? e.to_i : defaults[i] }

gen(*params).tap { |verts| verts.each { |v| puts("v #{v.join(' ')}") } }
  .each_with_index
  .map { |_, i| i + 1 }
  .then { |indices| puts("f #{([indices[0], indices[1]] + indices[1..].reverse).join(' ')}") }
