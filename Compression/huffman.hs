module Huffman where

import Data.List (sort, insert)
import qualified Data.Map as M
import Data.Function (on)
import Control.Arrow (second)

-- A Bit is either O or I (representing zero and one).
data Bit = O | I
           deriving (Ord, Eq)

-- Convert a single Bit to a Char (for use in printing)
toChar :: Bit -> Char
toChar O = '0'
toChar I = '1'

-- Make Bit an instance of typeclass Show, so that we can print Bits
-- and lists of Bits.
instance Show Bit where
         show b = toChar b : ""
         showList [] s = s
         showList (b:bs) s = (toChar b) : showList bs s

-- Typeclass 'HTree a' is a Huffman tree containing symbols of type
-- 'a'. For efficiency, we use Int (not Integer) as the weight.
data HTree a = Branch {zer :: (HTree a), one :: (HTree a),
                       wt  :: Int }
             | Leaf {symb :: a,  wt :: Int }
              deriving (Show)

-- Make 'HTree a' an instance of typeclass Ord, so that we can compare
-- 'HTree a' values. They are compared based on their weight.
instance Ord (HTree a) where
    compare = compare `on` wt

-- Make 'HTree a' an instance of typeclass Eq, so that we can compare
-- 'HTree a' values for equality. They are compared based on their
-- weight.
instance Eq (HTree a) where
    (==) = (==) `on` wt

-- If we were making 'HTree a' an instance of typeclass Monoid, we'd
-- call the function to add them 'mappend'. But we're not, so we
-- won't.
combineTree t1 t2 = Branch t1 t2 (wt t1 + wt t2)

buildDecTree :: [(a,Int)] -> HTree a
buildDecTree = build . sort . map (uncurry Leaf)
    where build (t:[])     = t
          build (t1:t2:ts) = build $ insert (t1 `combineTree` t2) ts

-- Typeclass 'CodeDict a' is a dictionary with keys that are of type
-- 'a' (our symbol type) and values that are of type [Bit].
type CodeDict a = M.Map a [Bit]

buildEncDict :: (Ord a) => HTree a -> CodeDict a
buildEncDict = M.fromList . build 
    where build (Leaf t _)     = (t,[]) : []
          build (Branch a b _) = mapBit O a ++ mapBit I b
           -- build up the codes in the snd of each Leaf's tuple:
          mapBit b = map (second (b:)) . build

encode :: (Ord a) => CodeDict a -> [a] -> [Bit]
encode d = concatMap (d M.!)

decode :: HTree a -> [Bit] -> [a]
decode t [] = []
decode t bs = dec bs t
    where dec bs' (Leaf x _) = x : decode t bs'
          dec (O:bs') (Branch l _ _) = dec bs' l 
          dec (I:bs') (Branch _ r _) = dec bs' r 


-- We seem to need to specify the type of *population*, so that
-- Haskell does not deduce the type to be [(Char,Integer)].

-- This population is the one for our "canonical 8 symbol (3-bit)
-- alphabet".
population = [('1',2), ('2',3), ('3',3), ('4',3),
              ('5',5), ('0',7), ('6',8), ('7',9)] :: [(Char,Int)]

huffmanTree' = buildDecTree population

encodePhrase phr = let dict = buildEncDict huffmanTree'
                   in encode dict phr

