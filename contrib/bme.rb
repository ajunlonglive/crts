#!/usr/bin/ruby --disable=all

# a super duper barebones bitmap glyph editor

require('io/console')


def write_char(arr)
  arr.each { |e| puts(sprintf("%08b", e).reverse.tr('0', '_') + " 0x#{e.to_s(16)}\e[K") }
end

def write_bitmap_entry(arr)
  puts(<<~HDOC)
  STARTCHAR <charname>
  ENCODING <encoding>
  SWIDTH 448 0
  DWIDTH 7 0
  BBX 7 15 0 -3
  BITMAP
  #{arr.map { |e| sprintf("%02x", e) }.join("\n")}
  ENDCHAR
  HDOC
end

def toggle_bit(arr, x, y)
  arr[y] ^= 1 << x
end

char = 15.times.map { 0 }

cursor = [0, 0]

print(%x(clear))

loop do
  print("\e[0;0H")
  write_char(char)

  sel = char[cursor[1]] & (1 << cursor[0]) != 0 ? '1' : '_'
  printf("\e[#{cursor[1] + 1};#{cursor[0] + 1}H\e[32m#{sel}\e[0m")

  case $stdin.getch
  when 'h'
    cursor[0] -= 1
  when 'l'
    cursor[0] += 1
  when 'j'
    cursor[1] += 1
  when 'k'
    cursor[1] -= 1
  when 't'
    toggle_bit(char, *cursor)
  when 'p'
    print(%x(clear))
    write_bitmap_entry(char)
    $stdin.getch
    print(%x(clear))
  when 'q'
    break
  end
end
