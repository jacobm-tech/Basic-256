;   B A S I C 2 5 6 . n s i  
 ;   M o d i f i c a t i o n   H i s t o r y  
 ;   d a t e . . . . . .   p r o g r a m m e r . . .   d e s c r i p t i o n . . .  
 ;   2 0 0 8 - 0 9 - 0 1   j . m . r e n e a u         o r i g i n a l   c o d i n g  
 ;   2 0 1 2 - 1 2 - 0 2   j . m . r e n e a u         c h a n g e d   t o   r e q u i r e   t h e   u n s i s   v e r s i o n   f o r   u n i c o d e   s u p p o r t  
 ;                                                     M o r e   i n f o r m a t i o n   a t   h t t p : / / w w w . s c r a t c h p a p e r . c o m /  
 ;   2 0 1 3 - 1 1 - 1 2   j . m . r e n e a u         m a j o r   r e w r i t e   f o r   1 . 0 . 0 . 0   a n d   c h a n g e   t o   Q T 5 . 1  
 ;  
  
 ! i n c l u d e   n s D i a l o g s . n s h  
  
 ! d e f i n e   V E R S I O N   " 1 . 0 . 0 . 7 "  
 ! d e f i n e   V E R S I O N D A T E   " 2 0 1 4 - 0 1 - 0 3 "  
 ! d e f i n e   S D K _ B I N   " C : \ Q t \ Q t 5 . 1 . 1 \ 5 . 1 . 1 \ m i n g w 4 8 _ 3 2 \ b i n "  
 ! d e f i n e   S D K _ L I B   " C : \ Q t \ Q t 5 . 1 . 1 \ 5 . 1 . 1 \ m i n g w 4 8 _ 3 2 \ l i b "  
 ! d e f i n e   S D K _ P L U G I N S   " C : \ Q t \ Q t 5 . 1 . 1 \ 5 . 1 . 1 \ m i n g w 4 8 _ 3 2 \ p l u g i n s "  
  
 v a r   c u s t o m D i a l o g  
 v a r   c u s t o m L a b e l 0  
 v a r   c u s t o m L a b e l 1  
 v a r   c u s t o m I m a g e  
 v a r   c u s t o m I m a g e H a n d l e  
  
 F u n c t i o n   . o n I n i t  
 F u n c t i o n E n d  
  
 F u n c t i o n   c u s t o m P a g e  
  
 	 n s D i a l o g s : : C r e a t e   / N O U N L O A D   1 0 1 8  
 	 P o p   $ c u s t o m D i a l o g  
  
 	 $ { I f }   $ c u s t o m D i a l o g   = =   e r r o r  
 	 	 A b o r t  
 	 $ { E n d I f }  
  
 	 $ { N S D _ C r e a t e B i t m a p }   0   0   1 0 0 %   1 0 0 %   " "  
 	 P o p   $ c u s t o m I m a g e  
 	 $ { N S D _ S e t I m a g e }   $ c u s t o m I m a g e   r e s o u r c e s \ i m a g e s \ b a s i c 2 5 6 . b m p   $ c u s t o m I m a g e H a n d l e  
  
 	 $ { N S D _ C r e a t e L a b e l }   5 0   0   8 0 %   1 0 %   " B A S I C 2 5 6   $ { V E R S I O N }   ( $ { V E R S I O N D A T E } ) "  
 	 P o p   $ c u s t o m L a b e l 0  
 	 $ { N S D _ C r e a t e L a b e l }   0   5 0   1 0 0 %   8 0 %   " T h i s   i n s t a l l e r   w i l l   l o a d   B A S I C 2 5 6 .     P r e v i o u s   v e r s i o n s   w i l l   b e   o v e r w r i t t e n   a n d   a n y   s a v e d   f i l e s   w i l l   b e   p r e s e r v e d . "  
 	 P o p   $ c u s t o m L a b e l 1  
  
 	 n s D i a l o g s : : S h o w  
 F u n c t i o n E n d  
  
  
 ;   T h e   n a m e   o f   t h e   i n s t a l l e r  
 N a m e   " B A S I C 2 5 6   $ { V E R S I O N }   ( $ { V E R S I O N D A T E } ) "  
  
 ;   T h e   f i l e   t o   w r i t e  
 O u t F i l e   B A S I C 2 5 6 - $ { V E R S I O N } _ W i n 3 2 _ I n s t a l l . e x e  
  
 ;   T h e   d e f a u l t   i n s t a l l a t i o n   d i r e c t o r y  
 I n s t a l l D i r   $ P R O G R A M F I L E S \ B A S I C 2 5 6  
  
 ;   R e g i s t r y   k e y   t o   c h e c k   f o r   d i r e c t o r y   ( s o   i f   y o u   i n s t a l l   a g a i n ,   i t   w i l l    
 ;   o v e r w r i t e   t h e   o l d   o n e   a u t o m a t i c a l l y )  
 I n s t a l l D i r R e g K e y   H K L M   " S o f t w a r e \ B A S I C 2 5 6 "   " I n s t a l l _ D i r "  
  
 ;   R e q u e s t   a p p l i c a t i o n   p r i v i l e g e s   f o r   W i n d o w s   V i s t a  
 R e q u e s t E x e c u t i o n L e v e l   a d m i n  
  
 I n s t T y p e   " F u l l "  
 I n s t T y p e   " M i n i m a l "  
 ; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
  
 ;   P a g e s  
  
 P a g e   c u s t o m   c u s t o m P a g e   " "   " :   B A S I C 2 5 6   W e l c o m e "  
 P a g e   l i c e n s e  
 L i c e n s e D a t a   " l i c e n s e . t x t "  
 P a g e   c o m p o n e n t s  
 P a g e   d i r e c t o r y  
 P a g e   i n s t f i l e s  
  
 U n i n s t P a g e   u n i n s t C o n f i r m  
 U n i n s t P a g e   i n s t f i l e s  
  
 ; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
  
 ;   T h e   s t u f f   t o   i n s t a l l  
 S e c t i o n   " B A S I C 2 5 6 "  
  
     S e c t i o n I n   1   2   R O  
      
     S e t O u t P a t h   $ I N S T D I R \ T r a n s l a t i o n s  
     S e t F i l e A t t r i b u t e s   $ I N S T D I R \ T r a n s l a t i o n s   H I D D E N  
     F i l e   . \ T r a n s l a t i o n s \ * . q m  
  
     S e t O u t P a t h   $ I N S T D I R \ e s p e a k - d a t a  
     S e t F i l e A t t r i b u t e s   $ I N S T D I R \ e s p e a k - d a t a   H I D D E N  
     F i l e   / r   / x   " . s v n "   . \ r e l e a s e \ e s p e a k - d a t a \ *  
  
     S e t O u t P a t h   $ I N S T D I R \ a c c e s s i b l e  
     S e t F i l e A t t r i b u t e s   $ I N S T D I R \ a c c e s s i b l e   H I D D E N  
     F i l e   " $ { S D K _ P L U G I N S } \ a c c e s s i b l e \ q t a c c e s s i b l e w i d g e t s . d l l "  
  
     S e t O u t P a t h   $ I N S T D I R \ i m a g e f o r m a t s  
     S e t F i l e A t t r i b u t e s   $ I N S T D I R \ i m a g e f o r m a t s   H I D D E N  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q g i f . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q i c o . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q j p e g . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q m n g . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q s v g . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q t g a . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q t i f f . d l l "  
     F i l e   " $ { S D K _ P L U G I N S } \ i m a g e f o r m a t s \ q w b m p . d l l "  
  
     S e t O u t P a t h   $ I N S T D I R \ p l a t f o r m s  
     S e t F i l e A t t r i b u t e s   $ I N S T D I R \ p l a t f o r m s   H I D D E N  
     F i l e   " $ { S D K _ P L U G I N S } \ p l a t f o r m s \ q w i n d o w s . d l l "  
  
     S e t O u t P a t h   $ I N S T D I R \ p r i n t s u p p o r t  
     S e t F i l e A t t r i b u t e s   $ I N S T D I R \ p r i n t s u p p o r t   H I D D E N  
     F i l e   " $ { S D K _ P L U G I N S } \ p r i n t s u p p o r t \ w i n d o w s p r i n t e r s u p p o r t . d l l "  
  
     S e t O u t P a t h   $ I N S T D I R  
      
     F i l e   . \ r e l e a s e \ B A S I C 2 5 6 . e x e  
     F i l e   C h a n g e L o g  
     F i l e   C O N T R I B U T O R S  
     F i l e   l i c e n s e . t x t  
  
     F i l e   " $ { S D K _ B I N } \ i c u d t 5 1 . d l l "  
     F i l e   " $ { S D K _ B I N } \ i c u i n 5 1 . d l l "  
     F i l e   " $ { S D K _ B I N } \ i c u u c 5 1 . d l l "  
     F i l e   " $ { S D K _ B I N } \ l i b g c c _ s _ d w 2 - 1 . d l l "  
     F i l e   " $ { S D K _ B I N } \ l i b s t d c + + - 6 . d l l "  
     F i l e   " $ { S D K _ B I N } \ l i b w i n p t h r e a d - 1 . d l l "  
  
     F i l e   " $ { S D K _ B I N } \ Q t 5 C o r e . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 G u i . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 M u l t i m e d i a . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 N e t w o r k . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 P r i n t S u p p o r t . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 S q l . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 W e b K i t . d l l "  
     F i l e   " $ { S D K _ B I N } \ Q t 5 W i d g e t s . d l l "  
  
     F i l e   " $ { S D K _ L I B } \ l i b e s p e a k . d l l "  
     F i l e   " $ { S D K _ L I B } \ l i b p o r t a u d i o - 2 . d l l "  
     F i l e   " $ { S D K _ L I B } \ l i b p t h r e a d - 2 . d l l "  
     F i l e   " $ { S D K _ L I B } \ s q l i t e 3 . d l l "  
     F i l e   " $ { S D K _ L I B } \ i n p o u t 3 2 . d l l "  
     F i l e   " $ { S D K _ L I B } \ I n s t a l l D r i v e r . e x e "  
  
   ;   W r i t e   t h e   i n s t a l l a t i o n   p a t h   i n t o   t h e   r e g i s t r y  
     W r i t e R e g S t r   H K L M   S O F T W A R E \ B A S I C 2 5 6   " I n s t a l l _ D i r "   " $ I N S T D I R "  
      
     ;   W r i t e   t h e   u n i n s t a l l   k e y s   f o r   W i n d o w s  
     W r i t e R e g S t r   H K L M   " S o f t w a r e \ M i c r o s o f t \ W i n d o w s \ C u r r e n t V e r s i o n \ U n i n s t a l l \ B A S I C 2 5 6 "   " D i s p l a y N a m e "   " B A S I C - 2 5 6 "  
     W r i t e R e g S t r   H K L M   " S o f t w a r e \ M i c r o s o f t \ W i n d o w s \ C u r r e n t V e r s i o n \ U n i n s t a l l \ B A S I C 2 5 6 "   " U n i n s t a l l S t r i n g "   ' " $ I N S T D I R \ u n i n s t a l l . e x e " '  
     W r i t e R e g D W O R D   H K L M   " S o f t w a r e \ M i c r o s o f t \ W i n d o w s \ C u r r e n t V e r s i o n \ U n i n s t a l l \ B A S I C 2 5 6 "   " N o M o d i f y "   1  
     W r i t e R e g D W O R D   H K L M   " S o f t w a r e \ M i c r o s o f t \ W i n d o w s \ C u r r e n t V e r s i o n \ U n i n s t a l l \ B A S I C 2 5 6 "   " N o R e p a i r "   1  
     W r i t e U n i n s t a l l e r   " u n i n s t a l l . e x e "  
      
 S e c t i o n E n d  
  
 ;   S t a r t   M e n u   S h r t c u t s   ( c a n   b e   d i s a b l e d   b y   t h e   u s e r )  
 S e c t i o n   " S t a r t   M e n u   S h o r t c u t s "  
     S e c t i o n I n   1  
     S e t O u t P a t h   $ I N S T D I R    
     C r e a t e D i r e c t o r y   " $ S M P R O G R A M S \ B A S I C 2 5 6 "  
     C r e a t e S h o r t C u t   " $ S M P R O G R A M S \ B A S I C 2 5 6 \ B A S I C 2 5 6 . l n k "   " $ I N S T D I R \ B A S I C 2 5 6 . e x e "   " "   " $ I N S T D I R \ B A S I C 2 5 6 . e x e "   0  
     C r e a t e S h o r t C u t   " $ S M P R O G R A M S \ B A S I C 2 5 6 \ U n i n s t a l l . l n k "   " $ I N S T D I R \ u n i n s t a l l . e x e "   " "   " $ I N S T D I R \ u n i n s t a l l . e x e "   0  
 S e c t i o n E n d  
  
 ;   O f f l i n e   H e l p   ( c a n   b e   d i s a b l e d   b y   t h e   u s e r   f o r   e a c h   l a n g u a g e )  
 S e c t i o n G r o u p   " O f f - l i n e   H e l p   a n d   D o c u m e n t a t i o n "  
     S e c t i o n   " D u t c h   ( n l ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ n l * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
     S e c t i o n   " E n g l i s h   ( e n ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ e n * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d    
     S e c t i o n   " F r e n c h   ( f r ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ f r * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
     S e c t i o n   " G e r m a n   ( d e ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ d e * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
     S e c t i o n   " P o r t u g u � s   ( p t ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ p t * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
     S e c t i o n   " R o m a n a   ( r o ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ r o * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
     S e c t i o n   " r u  CAA:89  ( r u ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ r u * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
     S e c t i o n   " E s p a � o l   ( e s ) "  
         S e c t i o n I n   1  
         S e t O u t P a t h   $ I N S T D I R \ h e l p  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ s t a r t . h t m l  
         F i l e   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ e s * . *  
         F i l e   / r   / x   " . s v n "   . \ w i k i h e l p \ h e l p \ l i b  
     S e c t i o n E n d  
 S e c t i o n G r o u p E n d  
  
 ;   E x a m p l e s   ( c a n   b e   d i s a b l e d   b y   t h e   u s e r )  
 S e c t i o n   " E x a m p l e   P r o g r a m s "  
     S e c t i o n I n   1  
     S e t O u t P a t h   $ I N S T D I R  
     F i l e   / r   / x   " . s v n "   E x a m p l e s  
 S e c t i o n E n d  
  
 ; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
  
 ;   U n i n s t a l l e r  
  
 S e c t i o n   " U n i n s t a l l "  
      
     ;   R e m o v e   r e g i s t r y   k e y s  
     D e l e t e R e g K e y   H K L M   " S o f t w a r e \ M i c r o s o f t \ W i n d o w s \ C u r r e n t V e r s i o n \ U n i n s t a l l \ B A S I C 2 5 6 "  
     D e l e t e R e g K e y   H K L M   S O F T W A R E \ B A S I C 2 5 6  
  
     ;   R e m o v e   f i l e s   a n d   u n i n s t a l l e r  
     D e l e t e   $ I N S T D I R \ * . e x e  
     D e l e t e   $ I N S T D I R \ * . d l l  
     D e l e t e   $ I N S T D I R \ C h a n g e L o g  
     D e l e t e   $ I N S T D I R \ C O N T R I B U T O R S  
     D e l e t e   $ I N S T D I R \ l i c e n s e . t x t  
     R M D i r   / r   $ I N S T D I R \ e s p e a k - d a t a  
     R M D i r   / r   $ I N S T D I R \ E x a m p l e s  
     R M D i r   / r   $ I N S T D I R \ h e l p  
     R M D i r   / r   $ I N S T D I R \ T r a n s l a t i o n s  
     R M D i r   / r   $ I N S T D I R \ a c c e s s i b l e  
     R M D i r   / r   $ I N S T D I R \ i m a g e f o r m a t s  
     R M D i r   / r   $ I N S T D I R \ p l a t f o r m s  
     R M D i r   / r   $ I N S T D I R \ p r i n t s u p p o r t  
  
     D e l e t e   $ I N S T D I R \ u n i n s t a l l . e x e  
  
     ;   R e m o v e   s h o r t c u t s ,   i f   a n y  
     D e l e t e   " $ S M P R O G R A M S \ B A S I C 2 5 6 \ * . * "  
  
     ;   R e m o v e   d i r e c t o r i e s   u s e d  
     R M D i r   " $ S M P R O G R A M S \ B A S I C 2 5 6 "  
     R M D i r   " $ I N S T D I R "  
  
 S e c t i o n E n d  
 