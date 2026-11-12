# Adicionar uma equação
<!-- keywords: equação, equações, matemática, fórmula, fração, raiz quadrada, expoente, índice superior, índice inferior, potência, soma, integral, grego, símbolo, inserir equação, alt+=, acento, matriz, parênteses, equações incorporadas, galeria, alterar uma equação, editar uma equação, vetor, pdf, jpub, ficheiro pub -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. No JeffPub, uma equação é um objeto na página, tal como uma imagem: pode movê-la, redimensioná-la, rodá-la e dar-lhe uma cor.

## Colocar uma equação na página

1. Clique em **Inserir > Texto > Equação**, ou prima **Alt+=**.
2. Escreva a equação numa só linha. Por baixo da caixa, vê como ficará. Clique em **OK**.

A equação fica no meio da página, selecionada. Para começar a partir de uma equação conhecida, clique na seta ao lado de **Equação** e escolha uma equação incorporada, como a fórmula quadrática ou a área de um círculo.

## Como escrever

- **Frações:** escreva uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como um único elemento.
- **Potências e índices:** `x^2` é x ao quadrado, e `x_i` é x com um pequeno i em baixo. Pode usar ambos, como em `x_i^2`.
- **Raízes:** escreva `sqrt(x+1)` para uma raiz quadrada, ou `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** escreva `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. A parte depois do sublinhado fica em baixo, e a parte depois do acento circunflexo fica em cima.
- **Símbolos:** escreva uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤, e `\times` é ×. Funcionam cerca de 70 nomes.
- **Acentos:** `\hat{x}`, `\vec{v}` e `\bar{x}` colocam uma marca por cima da letra.
- **Parênteses que crescem:** escreva `\left( ... \right)`, e os parênteses crescem até à altura do que está entre eles.
- **Matrizes:** uma **matriz** é uma grelha de números. Escreva `\pmatrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha, e duas barras invertidas entre as linhas.

## Alterar uma equação

Selecione a equação e prima Enter, faça duplo clique nela, ou clique em **Inserir > Texto > Equação**. A caixa abre com a equação escrita numa só linha. Altere-a e clique em **OK**. Basta um passo para a anular.

Uma equação que um ficheiro trouxe com partes que a caixa não consegue escrever é mostrada tal como está. Para a alterar, escreva uma nova.

## Mover, redimensionar e colorir

Arraste a equação para a mover. Arraste um canto para a aumentar ou diminuir; a equação mantém as suas proporções. Use **Preenchimento da Forma** para alterar a cor das letras.

## Guardar

- **.jpub** mantém a equação, para que possa voltar a editá-la.
- **PDF e impressão** desenham a equação como formas, por isso fica nítida a qualquer tamanho.
- **.pub** não tem equações. O JeffPub guarda cada uma como uma imagem dela, no mesmo local e com o mesmo tamanho, e avisa-o quando guarda.
- **EPUB** e **HTML** guardam a equação como a linha que escreveu.
