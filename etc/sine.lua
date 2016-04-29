for i = 0, 255 do
  local o = i * math.pi * 2 / 256
  local s = math.sin( o )
  
  s = math.floor( ( s + 1 ) * 256 / 2 )
  s = math.min( s, 255 )
  s = math.max( s, 0 )
  
  io.write( string.format( '0x%02x, ', math.floor( s ) ) )
  
  if ( ( i + 1 ) % 16 ) == 0 then
    io.write( '\n' )
  end
end
