import Huffman

reportEncoding s = do
               putStr s
               putStr ": "
               putStrLn . show $ encodePhrase s

main = do
     reportEncoding "a"
     reportEncoding "b"
     reportEncoding "c"
     reportEncoding "d"
     reportEncoding "e"
     reportEncoding "abcde"
     -- reportEncoding "0"
     -- reportEncoding "1"
     -- reportEncoding "2"
     -- reportEncoding "3"
     -- reportEncoding "4"
     -- reportEncoding "5"
     -- reportEncoding "6"
     -- reportEncoding "7"
     -- reportEncoding "55274630607402075163066705471072677564736"
     -- reportEncoding "01234567"
