;;; Directory Local Variables
;;; For more information see (info "(emacs) Directory Variables")

(
 ;; For all modes
 (nil
  (indent-tabs-mode))
 ;; For c++-mode
 (c++-mode
  (fill-column . 80)
  (whitespace-line-column . 80)
  (c-file-style . "bsd")
  (eval ignore-errors
        (require 'whitespace)
        (whitespace-mode 0)
        (whitespace-mode 1))))
