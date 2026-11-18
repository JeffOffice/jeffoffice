# Adicionar uma equação
<!-- keywords: guia equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, expoente, índice, sobrescrito, subscrito, soma, integral, grego, símbolo, inserir equação, alt+=, acento, matriz, colchetes, equações prontas, galeria, alterar equação, editar equação, vetor, pdf, jpub, arquivo pub -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. No JeffPub, uma equação é um objeto na página, como uma imagem: você pode movê-la, redimensioná-la, girá-la e dar a ela uma cor.

## Colocar uma equação na página

1. Clique em **Inserir > Texto > Equação**, ou pressione **Alt+=**.
2. Digite a equação em uma linha. Abaixo da caixa, você vê como ela ficará. Clique em **OK**.

A equação é colocada no meio da página, selecionada. Para começar com uma equação conhecida, clique na seta ao lado de **Equação** e escolha uma equação pronta, como a fórmula quadrática ou a área de um círculo.

## Como digitar

- **Frações:** digite uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma parte.
- **Potências e subscritos:** `x^2` é x ao quadrado, e `x_i` é x com um i pequeno embaixo. Você pode usar os dois, como em `x_i^2`.
- **Raízes:** digite `sqrt(x+1)` para uma raiz quadrada, ou `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** digite `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. A parte depois do sublinhado vai embaixo, e a parte depois do acento circunflexo vai em cima.
- **Símbolos:** digite uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤, e `\times` é ×. Cerca de 70 nomes funcionam.
- **Acentos:** `\hat{x}`, `\vec{v}` e `\bar{x}` colocam uma marca sobre a letra.
- **Colchetes que crescem:** digite `\left( ... \right)`, e os colchetes crescem até a altura do que está entre eles.
- **Matrizes:** uma **matriz** é uma grade de números. Digite `\pmatrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha, e duas barras invertidas entre as linhas.

## Digitar em UnicodeMath ou LaTeX

Ao lado da caixa há uma lista de formatos de digitação: **Linha simples** (a descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática como texto simples, que muitas pessoas já conhecem. Quando você escolhe outro formato, o que foi digitado é escrito novamente nele.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um subscrito, `√(x)` é uma raiz, e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e elas ficam como texto simples em vez de letras de matemática. `(a|b)` cria colchetes com duas partes, e `\matrix(a&b@c&d)` cria uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** mudam a forma como a caixa mostra a sua equação. Profissional a desenha montada, como ela ficará. Linear mostra apenas a linha que você digitou. Se uma equação não puder ser escrita no formato escolhido, a caixa avisa você e mantém o que você digitou.

## Alterar uma equação

Selecione a equação e pressione Enter, ou dê um clique duplo nela, ou clique em **Inserir > Texto > Equação**. A caixa abre com a equação escrita em uma linha. Altere-a e clique em **OK**. Desfazer é uma única etapa.

Uma equação trazida por um arquivo abre em UnicodeMath, ou em LaTeX quando o UnicodeMath não consegue escrevê-la. Se nenhum dos dois conseguir, a caixa começa com o texto da equação. Clique em **OK** sem alterar nada, ou em **Cancelar**, para manter a equação como está, ou digite uma nova para substituí-la.

## A guia Equação

Selecione uma equação, e uma guia chamada **Equação** aparece no final da faixa de opções. Ela desaparece quando você seleciona outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** mudam a forma como a equação selecionada é mostrada na página: Profissional a desenha montada, e Linear mostra a linha em que foi digitada. A escolha fica guardada com a equação no arquivo `.jpub`, por isso é a mesma quando você abre o arquivo novamente. Um arquivo `.pub` recebe a imagem da equação como ela aparece na página. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Na próxima vez que você editar uma equação, ela será escrita no formato escolhido. Uma equação que nenhuma linha do tipo simples consegue escrever, como uma soma com os limites em cima e embaixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, conjuntos e lógica. Escolha um símbolo, e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, script, radical, integral, operador grande, colchete, função, acento, limite, operador e matriz. Clique em um botão para a forma usual, ou na seta ao lado dele para mais formas, como uma derivada, uma raiz com índice ou uma matriz identidade.

Uma estrutura é inserida no final da linha, com letras de exemplo, e a primeira é selecionada para que você digite por cima. A caixa tem as mesmas estruturas em uma fileira de pequenos botões acima da linha. Ali, a estrutura é inserida no cursor, e, se parte da linha estiver selecionada, essa parte vai para o primeiro lugar da estrutura.

## Mover, redimensionar e colorir

Arraste a equação para movê-la. Arraste um canto para aumentá-la ou diminuí-la; a equação mantém suas proporções. Use **Preenchimento da Forma** para mudar a cor das letras.

## Salvar

- **.jpub** mantém a equação, para que você possa editá-la novamente.
- **PDF e impressão** desenham a equação como formas, por isso ela continua nítida em qualquer tamanho.
- **.pub** não tem equações. O JeffPub salva cada uma como uma imagem dela, no mesmo lugar e com o mesmo tamanho, e avisa você ao salvar.
- **EPUB** e **HTML** levam a equação como a linha que você digitou.
