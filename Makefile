all: pcm
# on écrase la variable make CXX pour notre compilo
CXX=		clang++
# on veut que clang émette du code pour notre CPU seulement
CXXFLAGS+=	-march=native
# à la dernière version complètement supportée du standard ISO: https://clang.llvm.org/cxx_status.html
CXXFLAGS+=	-std=c++17
# avec les symboles de debugging pour pouvoir ouvrir notre binaire dans un débugger
CXXFLAGS+=	-g
# sans aucune optimisations, parce que c'est pas marrant de debug des binaires optimisés qui inlinent des loops
CXXFLAGS+=	-O
# on active les warnings
CXXFLAGS+=	-W
# tous les warnings
CXXFLAGS+=	-Wall
# TOUS les warnings
CXXFLAGS+=      -Wextra
# même les conversions sans cast sont des erreurs 
CXXFLAGS+=	-Wconversion -Wsign-conversion
# et tous les warnings sont traités comme des erreurs
CXXFLAGS+=	-Werror
