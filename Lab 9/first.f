c234567
       PROGRAM looper
       REAL LOTS(10,12)


       DO 220 I=1,10
              DO 110 J=1,12
                     LOTS(I,J) = 6.02E5
 110          CONTINUE
 220   CONTINUE
       WRITE(*, 140) LOTS(4, 9)
 140   FORMAT(1F10.2)    
              
       END PROGRAM looper