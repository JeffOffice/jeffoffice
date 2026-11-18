# Equação

<!-- keywords: guia equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, sobrescrito, subscrito, expoente, soma, integral, grego, símbolo, inserir equação, acento, matriz, colchetes, equações predefinidas, galeria, alterar equação, editar equação, ods, imagem, pdf -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. No JeffCrunch, uma equação é um objeto sobre as células, como uma imagem ou uma caixa de texto. Ela não é uma célula: não altera nenhum número da planilha.

## Colocar uma equação na planilha

1. Clique em **Inserir > Símbolos > Equação**.
2. Digite a equação em uma linha. Embaixo da caixa, você vê como ela ficará. Clique em **OK**.

A equação é colocada na célula em que você está, já selecionada. Para começar com uma conhecida, clique em **Equações Predefinidas** e escolha uma, como a fórmula de Bhaskara (fórmula quadrática) ou a área de um círculo.

## Como digitar

- **Frações:** digite uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma parte.
- **Potências e subscritos:** `x^2` é x ao quadrado, e `x_i` é x com um i pequeno embaixo.
- **Raízes:** digite `sqrt(x+1)` para uma raiz quadrada.
- **Somas, integrais e produtos:** digite `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`.
- **Símbolos:** digite uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, e `\leq` é ≤.
- **Acentos, colchetes e matrizes:** `\hat{x}`, `\left( ... \right)` e `\pmatrix{a & b \\ c & d}`.

## Digitar em UnicodeMath ou LaTeX

Ao lado da caixa há uma lista de formatos de digitação: **Linha simples** (a descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática como texto simples, que muitas pessoas já conhecem. Quando você escolhe outro formato, o que foi digitado é escrito novamente nele.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um subscrito, `√(x)` é uma raiz, e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e elas permanecem como texto simples, em vez de letras matemáticas. `(a|b)` cria colchetes com duas partes, e `\matrix(a&b@c&d)` cria uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** mudam a forma como a caixa mostra sua equação. Profissional a desenha montada, como ficará. Linear mostra apenas a linha que você digitou. Se uma equação não puder ser escrita no formato escolhido, a caixa avisa e mantém o que você digitou.

## A guia Equação

Ao selecionar uma equação, uma guia chamada **Equação** aparece no final da faixa de opções. Ela some quando você seleciona outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** escolhem como a caixa começa: montada ou apenas a linha. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Na próxima vez que você editar uma equação, ela será escrita no formato escolhido. Uma equação que nenhuma linha do tipo simples consegue escrever, como uma soma com os limites em cima e embaixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, e conjuntos e lógica. Escolha um símbolo, e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, script, radical, integral, operador grande, colchete, função, acento, limite, operador e matriz. Clique em um botão para sua forma usual, ou na seta ao lado para mais formas, como uma derivada, uma raiz com índice ou uma matriz identidade.

Uma estrutura é inserida no fim da linha, com letras de exemplo, e a primeira fica selecionada para que você digite por cima. A caixa tem as mesmas estruturas em uma fileira de pequenos botões acima da linha. Ali, a estrutura é inserida no cursor e, se parte da linha estiver selecionada, essa parte vai para o primeiro espaço da estrutura.

## Alterar, mover e redimensionar

Clique duas vezes na equação, ou selecione-a e clique em **Inserir > Símbolos > Equação**, para abrir a caixa com a sua linha. Altere-a e clique em **OK**. Basta uma etapa para desfazer. Arraste a equação para movê-la, e arraste uma alça para dimensioná-la; a equação mantém suas proporções dentro da caixa.

## Salvar

- **.xlsx** mantém a equação como equação, na forma padrão para matemática, com uma imagem ao lado para programas que não conseguem ler matemática. Ao abrir o arquivo novamente, a equação aparece. Uma equação feita em outro programa é mostrada da mesma forma; se a matemática dela não puder ser lida, você verá a imagem dela.
- **PDF e impressão** desenham a equação como formas, para que ela fique nítida em qualquer tamanho.
- **Cópias em .ods** não têm equações. O JeffCrunch salva cada uma como uma imagem dela, no mesmo lugar, e cita as equações quando informa o que a cópia deixa de fora.
