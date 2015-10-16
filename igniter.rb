#!/usr/bin/ruby
if (ARGV.size != 2)
  print "wrong number of args: #{ARGV.size}\n"
  print "usage igniter <m> <n>\n"
  exit
end
m = ARGV[0].to_i
n = ARGV[1].to_i


print "#{m} #{n}\n"
m.times do |i|
  n.times do |j|
    printf "%.1f ", rand
  end
  print "\n"
end

print "#{n}\n"
n.times do |j|
  printf "%.1f ", rand
end
print "\n"
