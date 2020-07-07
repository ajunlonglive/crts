#!/usr/bin/ruby --disable=all

# generates indices to render chunk mesh

d = 18

(d * d).times.map { |i|
  unless (i + 1) % d == 0 || i > 271
    nil
  else
    [[i + 1 + d, i + 1, i], [i + d, i + 1 + d, i]]
  end
  }
  .compact
  .flatten(1)
  .each { |(a, b, c)| printf("%d, %d, %d,\n", a, b, c) }
