# c_water

## dependencies

Il y a un flake.nix a la root

```bash
nix develop
```

## usage

```bash
make clean all && ./c_water
```

## configuration

On peut modifier la configuration dans config/config.c, pas besoin de recompiler apres :]
Ou dans la simulation avec les fleches de gauche et droite, et haut bas

Pour les parametres du rendu raymarching, il y a plein de constantes dans shaders/shader.frag
