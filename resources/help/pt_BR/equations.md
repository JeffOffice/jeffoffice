# Adicionar uma equação
<!-- keywords: equação, equações, matemática, fórmula, fração, raiz quadrada, sobrescrito, subscrito, expoente, soma, integral, grego, símbolo, inserir equação, alt+=, acento, matriz, colchetes, equações prontas, galeria, alterar uma equação, editar uma equação, vetor, pdf, jpub, arquivo pub -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. No JeffPub, uma equação é um objeto na página, como uma imagem: você pode movê-la, redimensioná-la, girá-la e dar-lhe uma cor.

## Colocar uma equação na página

1. Clique em **Inserir > Texto > Equação**, ou pressione **Alt+=**.
2. Digite a equação em uma linha. Abaixo da caixa, você vê como ela ficará. Clique em **OK**.

A equação é colocada no meio da página, já selecionada. Para começar com uma equação conhecida, clique na seta ao lado de **Equação** e escolha uma equação pronta, como a fórmula quadrática ou a área de um círculo.

## Como digitar

- **Frações:** digite uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma única parte.
- **Potências e subscritos:** `x^2` é x elevado ao quadrado, e `x_i` é x com um i pequeno embaixo. Você pode usar os dois, como em `x_i^2`.
- **Raízes:** digite `sqrt(x+1)` para uma raiz quadrada, ou `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** digite `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. A parte depois do sublinhado fica embaixo, e a parte depois do acento circunflexo fica em cima.
- **Símbolos:** digite uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤ e `\times` é ×. Cerca de 70 nomes funcionam.
- **Acentos:** `\hat{x}`, `\vec{v}` e `\bar{x}` colocam uma marca sobre a letra.
- **Colchetes que crescem:** digite `\left( ... \right)`, e os colchetes crescem até a altura do que está entre eles.
- **Matrizes:** uma **matriz** é uma grade de números. Digite `\pmatrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha e duas barras invertidas entre as linhas.

## Alterar uma equação

Selecione a equação e pressione Enter, ou clique duas vezes nela, ou clique em **Inserir > Texto > Equação**. A caixa abre com a equação escrita em uma linha. Altere-a e clique em **OK**. Basta uma etapa para desfazer.

Uma equação que um arquivo trouxe com partes que a caixa não consegue escrever é mostrada como está. Para alterá-la, digite uma nova.

## Mover, redimensionar e colorir

Arraste a equação para movê-la. Arraste um canto para aumentá-la ou diminuí-la; a equação mantém suas proporções. Use **Preenchimento da Forma** para mudar a cor das letras.

## Salvamento

- **.jpub** mantém a equação, para que você possa editá-la novamente.
- **PDF e impressão** desenham a equação como formas, de modo que ela continua nítida em qualquer tamanho.
- **.pub** não tem equações. O JeffPub salva cada uma como uma imagem dela, no mesmo lugar e tamanho, e avisa você ao salvar.
- **EPUB** e **HTML** guardam a equação como a linha que você digitou.
