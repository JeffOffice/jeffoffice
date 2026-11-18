# Equações

<!-- keywords: guia equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, sobrescrito, subscrito, expoente, soma, integral, grego, símbolo, inserir equação, acento, matriz, colchetes, equações predefinidas, galeria, alterar uma equação, editar uma equação, pptx, odp, imagem, pdf, apresentação de slides, imprimir -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. Em um slide, a equação é um objeto, como uma imagem ou uma caixa de texto. Você pode movê-la, redimensioná-la, girá-la, copiá-la e excluí-la.

## Inserir uma equação no slide

1. Clique em **Inserir > Símbolos > Equação** (ou pressione **Alt+=**).
2. Digite a equação em uma linha. Abaixo da caixa você vê como ela ficará. Clique em **OK**.

A equação é colocada no meio do slide, já selecionada. Suas letras têm mais ou menos a altura de um título. Uma equação muito larga é reduzida para caber no slide. Para começar com uma conhecida, clique em **Equações Predefinidas** e escolha uma, como a fórmula de Bhaskara ou a área de um círculo.

## Como digitar

- **Frações:** digite uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma parte.
- **Potências e subscritos:** `x^2` é x ao quadrado, e `x_i` é x com um i pequeno embaixo.
- **Raízes:** digite `sqrt(x+1)` para uma raiz quadrada.
- **Somas, integrais e produtos:** digite `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`.
- **Símbolos:** digite uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, e `\leq` é ≤.
- **Acentos, colchetes e matrizes:** `\hat{x}`, `\left( ... \right)` e `\pmatrix{a & b \\ c & d}`.

## Digitar em UnicodeMath ou LaTeX

Ao lado da caixa há uma lista de formatos de digitação: **Linha simples** (a descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática como texto simples, que muitas pessoas já conhecem. Ao escolher outro formato, o que você digitou é escrito novamente nele.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um subscrito, `√(x)` é uma raiz, e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e elas permanecem como texto simples em vez de letras matemáticas. `(a|b)` faz colchetes com duas partes, e `\matrix(a&b@c&d)` faz uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** mudam a forma como a caixa mostra sua equação. Profissional a desenha montada, como ficará. Linear mostra apenas a linha que você digitou. Se uma equação não puder ser escrita no formato escolhido, a caixa avisa e mantém o que você digitou.

## Alterar, mover e redimensionar

Clique duas vezes na equação, ou selecione-a e clique em **Inserir > Símbolos > Equação**, para abrir a caixa com a sua linha. Altere-a e clique em **OK**. Ela permanece onde está e mantém o tamanho, e a alteração pode ser desfeita em uma etapa. Arraste a equação para movê-la. Arraste uma alça para redimensioná-la; a equação mantém suas proporções dentro da caixa.

Uma equação que veio de um arquivo também pode ser alterada. Enquanto você não a alterar, o JeffDeck mostra a imagem que o arquivo guarda e a salva de volta exatamente como estava. Clique duas vezes nela (ou selecione-a e clique em **Equação**). A caixa começa com uma linha que descreve a equação. Se você alterar a linha, a imagem se torna uma equação, no mesmo lugar, e o JeffDeck monta a matemática novamente a partir do que você digitou, então quaisquer fontes especiais da matemática antiga são perdidas. Se você clicar em **OK** sem alterar a linha, nada muda.

## A guia Equação

Selecione uma equação, e uma guia chamada **Equação** aparece no fim da faixa de opções. Ela some quando você seleciona outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** mudam a forma como a equação selecionada é mostrada no slide: Profissional a desenha montada, e Linear mostra a linha em que foi digitada. A escolha fica com a equação apenas enquanto a apresentação estiver aberta: um `.pptx` ou `.odp` salvo sempre guarda a equação montada, e por isso ela se abre assim novamente. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Na próxima vez que você editar uma equação, ela será escrita no formato escolhido. Uma equação que nenhuma linha do tipo simples consegue escrever, como uma soma com seus limites acima e abaixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, conjuntos e lógica. Escolha um símbolo, e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, script, radical, integral, operador grande, colchete, função, acento, limite, operador e matriz. Clique em um botão para a forma usual, ou na seta ao lado para mais formas, como uma derivada, uma raiz com índice ou uma matriz identidade.

Uma estrutura é inserida no fim da linha, com letras de exemplo, e a primeira fica selecionada para que você possa digitar por cima dela. A caixa tem as mesmas estruturas em uma fileira de pequenos botões acima da linha. Ali, a estrutura é inserida no cursor, e, se parte da linha estiver selecionada, essa parte vai para o primeiro lugar da estrutura.

## Apresentação de slides, impressão e PDF

A apresentação de slides, a impressão e os arquivos PDF desenham a equação como formas, por isso ela permanece nítida em qualquer tamanho.

## Salvamento

- **.pptx** mantém a equação como equação, na forma padrão para matemática, com uma imagem dela ao lado para programas que não conseguem ler matemática. Ao abrir o arquivo novamente, o JeffDeck mostra a imagem e mantém a matemática, e você ainda pode alterá-la como descrito acima. Uma apresentação comporta no máximo 2.000 equações e 16 MB de matemática no total. Além disso, o JeffDeck não adiciona mais nenhuma.
- **.odp** não tem equações na cópia do JeffDeck. Cada uma é salva como uma imagem dela, no mesmo lugar, e a mensagem após o salvamento informa isso.
- Uma caixa de texto com palavras ao redor de sua matemática não é uma equação aqui. Ela é mostrada como a imagem que o arquivo guarda e salva de volta inteira.

## Veja também

- [Formas, imagens, tabelas e Texto Artístico](objects)
- [Guia Inserir](tab-insert)
