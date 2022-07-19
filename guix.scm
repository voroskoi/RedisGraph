;; What follows is a "manifest" equivalent to the command line you gave.
;; You can store it in a file that you may then pass to any 'guix' command
;; that accepts a '--manifest' (or '-m') option.

(concatenate-manifests
  (list (specifications->manifest
          (list
            "zig@0.10"
            "zls"
            "peg"
            "graphblas"
            "redisearch"
            "libcypher-parser"
            "xxhash"))
        (package->development-manifest
          (specification->package "redisgraph"))))
