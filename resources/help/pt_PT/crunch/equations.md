# Equações

<!-- keywords: separador equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, expoente, índice superior, índice inferior, potência, soma, integral, grego, símbolo, inserir equação, acento, matriz, parênteses, equações incorporadas, galeria, alterar uma equação, editar uma equação, ods, imagem, pdf -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. No JeffCrunch, uma equação é um objeto sobre as células, como uma imagem ou uma caixa de texto. Não é uma célula: não altera nenhum número da folha.

## Colocar uma equação na folha

1. Clique em **Inserir > Símbolos > Equação**.
2. Escreva a equação numa só linha. Por baixo da caixa, vê o aspeto que terá. Clique em **OK**.

A equação é colocada na célula onde está, já selecionada. Para começar a partir de uma conhecida, clique em **Equações Incorporadas** e escolha uma, como a fórmula quadrática ou a área de um círculo.

## Como escrever

- **Frações:** escreva uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma parte.
- **Potências e índices:** `x^2` é x ao quadrado, e `x_i` é x com um i pequeno em baixo.
- **Raízes:** escreva `sqrt(x+1)` para uma raiz quadrada.
- **Somas, integrais e produtos:** escreva `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`.
- **Símbolos:** escreva uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, e `\leq` é ≤.
- **Acentos, parênteses e matrizes:** `\hat{x}`, `\left( ... \right)` e `\pmatrix{a & b \\ c & d}`.

## Escrever em UnicodeMath ou LaTeX

Ao lado da caixa há uma lista de formatos de escrita: **Linha simples** (a descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática em texto simples, que muitas pessoas já conhecem. Quando escolhe outro formato, o que escreveu é reescrito nele.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um índice, `√(x)` é uma raiz, e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e ficam como texto simples em vez de letras matemáticas. `(a|b)` cria parênteses com duas partes, e `\matrix(a&b@c&d)` cria uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** alteram a forma como a caixa mostra a equação. Profissional desenha-a em camadas, tal como ficará. Linear mostra apenas a linha que escreveu. Se uma equação não puder ser escrita no formato escolhido, a caixa avisa e mantém o que escreveu.

## O separador Equação

Ao selecionar uma equação, aparece no fim do friso um separador chamado **Equação**. Desaparece quando seleciona outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** escolhem como a caixa abre, em camadas ou apenas com a linha. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Da próxima vez que editar uma equação, ela fica escrita no formato escolhido. Uma equação que nenhuma linha do tipo simples consegue escrever, como uma soma com os limites em cima e em baixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, e conjuntos e lógica. Escolha um símbolo, e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, índice, radical, integral, operador grande, parêntese, função, acento, limite, operador e matriz. Clique num botão para a forma habitual, ou na seta ao lado para mais formas, como uma derivada, uma raiz com índice ou uma matriz identidade.

Uma estrutura é inserida no fim da linha, com letras de exemplo, e a primeira fica selecionada para que possa escrever por cima. A caixa tem as mesmas estruturas numa fila de pequenos botões acima da linha. Aí, a estrutura é inserida no cursor, e se parte da linha estiver selecionada, essa parte passa para o primeiro lugar da estrutura.

## Alterar, mover e dimensionar

Faça duplo clique na equação, ou selecione-a e clique em **Inserir > Símbolos > Equação**, para abrir a caixa com a respetiva linha. Altere-a e clique em **OK**. Desfazer é um só passo. Arraste a equação para a mover, e arraste uma alça para a dimensionar; a equação mantém as proporções dentro da caixa.

## Guardar

- **.xlsx** mantém a equação como equação, na forma-padrão para matemática, com uma imagem ao lado para programas que não conseguem ler matemática. Ao abrir o ficheiro de novo, a equação volta a aparecer. Uma equação criada noutro programa é apresentada da mesma forma; se a sua matemática não puder ser lida, vê a respetiva imagem.
- **PDF e impressão** desenham a equação como formas, pelo que se mantém nítida em qualquer tamanho.
- As cópias **.ods** não têm equações. O JeffCrunch guarda cada uma como uma imagem, no mesmo lugar, e indica o nome das equações quando lhe diz o que a cópia não inclui.
