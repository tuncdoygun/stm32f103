        MODULE  ?addint

        PUBWEAK _addint

        SECTION .deneme:CODE
; Giriþte par1: R0 par2: R1
; R0 = R0 + R1
; Geri dönüþ: R0
_addint:
        add     R0, R0, R1
        BX      LR
        
        END
