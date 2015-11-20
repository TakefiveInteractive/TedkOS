# coding: utf-8
# USAGE: ruby ttf2png.rb

#//-------------------------------------------
# Note:
#//-------------------------------------------

# Unicode(code point):
# U+3042 == "あ" == "\u3042" == \u{3042}

# How to get code point:
# "あ".unpack( "U*" ).first.to_s(16)  #=> "3042"
# "あ".encode( "UTF-8" ).ord.to_s(16) #=> "3042"

#//-------------------------------------------
# Config:
#//-------------------------------------------

FONT      = "inconsolata.ttf"
FONT_SIZE = 36 #pt

FIRST     = " "  # == "あ"
LAST      = "~"  # == "ヺ"

#//-------------------------------------------

(FIRST..LAST).step do |a|
  if a == "\\"
  	a = "\\\\"
  end
  if a == "@"
  	a = "\\@"
  end
  if a == "'"
  	cmd = "convert -background none -fill black -font #{FONT} -pointsize #{FONT_SIZE} label:\"#{a}\" #{FONT_SIZE}_#{a.ord}.png"
	puts cmd
	system(cmd) if ARGV[0]
  else
  	cmd = "convert -background none -fill black -font #{FONT} -pointsize #{FONT_SIZE} label:'#{a}' #{FONT_SIZE}_#{a.ord}.png"
	puts cmd
	system(cmd) if ARGV[0]
  end
end

print sprintf("If you want to execute these commands, try this: \nruby %s 1\n", File.basename(__FILE__)) if ARGV.length == 0
