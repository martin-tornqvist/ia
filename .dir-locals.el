(
 ;; For all modes
 (nil
  . (
     (indent-tabs-mode . nil)
     ))
 ;; For c++ mode
 (c++-mode
  . (
     (fill-column . 80)
     ;; NOTE: This will not have an effect until whitespace-mode is (re)started.
     (whitespace-line-column . 80)
     (c-file-style . "bsd")
     ;; Disable/enable whitespace-mode, see note above.
     ;; NOTE: This is "unsafe", see:
     ;; M-x describe-variable -> enable-local-variables
     ;; This will not be evaluated if enable-local-variables is set to ":safe".
     (eval ignore-errors
           (require 'whitespace)
           (whitespace-mode 0)
           (whitespace-mode 1)))
  )
 )
