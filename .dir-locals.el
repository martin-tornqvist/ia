((nil
  . ((eval . (progn
               (require 'projectile)
               (setq company-clang-arguments (delete-dups (append
                                                           company-clang-arguments
                                                           (list
                                                            "-std=c++14"
                                                            (concat "-I" (projectile-project-root) "include")
                                                            (concat "-I" (projectile-project-root) "rl_utils/include")

                                                            "-I/usr/include/SDL2"
                                                            ))))))))
 (c++-mode
  (whitespace-line-column . 80)
  (eval ignore-errors
        (add-hook 'write-contents-functions
                  (lambda ()
                    (delete-trailing-whitespace)
                    nil))
        (require 'whitespace)
        (whitespace-mode 0)
        (whitespace-mode 1)))
 )
