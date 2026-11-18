# Adicionar uma equação
<!-- keywords: separador equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, expoente, índice superior, índice inferior, soma, integral, grego, símbolo, inserir equação, alt+=, acento, matriz, parênteses, equações incorporadas, galeria, alterar uma equação, editar uma equação, vetor, pdf, jpub, ficheiro pub -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. No JeffPub, uma equação é um objeto na página, tal como uma imagem: pode movê-la, redimensioná-la, rodá-la e dar-lhe uma cor.

## Colocar uma equação na página

1. Clique em **Inserir > Texto > Equação**, ou prima **Alt+=**.
2. Escreva a equação numa única linha. Por baixo da caixa vê o aspeto que terá. Clique em **OK**.

A equação fica no centro da página, selecionada. Para partir de uma já conhecida, clique na seta ao lado de **Equação** e escolha uma equação incorporada, como a fórmula quadrática ou a área de um círculo.

## Como escrever

- **Frações:** escreva uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma única parte.
- **Potências e índices:** `x^2` é x ao quadrado, e `x_i` é x com um i pequeno em baixo. Pode usar ambos, como em `x_i^2`.
- **Raízes:** escreva `sqrt(x+1)` para uma raiz quadrada, ou `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** escreva `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. A parte a seguir ao sublinhado fica em baixo, e a parte a seguir ao acento circunflexo fica em cima.
- **Símbolos:** escreva uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤ e `\times` é ×. Funcionam cerca de 70 nomes.
- **Acentos:** `\hat{x}`, `\vec{v}` e `\bar{x}` colocam uma marca sobre a letra.
- **Parênteses que crescem:** escreva `\left( ... \right)`, e os parênteses crescem até à altura do que está entre eles.
- **Matrizes:** uma **matriz** é uma grelha de números. Escreva `\pmatrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha, e duas barras invertidas entre as linhas.

## Escrever em UnicodeMath ou LaTeX

Ao lado da caixa há uma lista de formatos de escrita: **Linha Simples** (a descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática como texto simples, que muitas pessoas já conhecem. Quando escolhe outro formato, o que escreveu é reescrito nesse formato.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um índice, `√(x)` é uma raiz, e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e ficam como texto simples em vez de letras matemáticas. `(a|b)` cria parênteses com duas partes, e `\matrix(a&b@c&d)` cria uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** alteram a forma como a caixa mostra a equação. Profissional desenha-a em camadas, tal como ficará. Linear mostra apenas a linha que escreveu. Se uma equação não puder ser escrita no formato escolhido, a caixa indica-o e mantém o que escreveu.

## Alterar uma equação

Selecione a equação e prima Enter, faça duplo clique nela, ou clique em **Inserir > Texto > Equação**. A caixa abre com a equação escrita numa linha. Altere-a e clique em **OK**. Basta um passo para a anular.

Uma equação trazida por um ficheiro abre em UnicodeMath, ou em LaTeX quando o UnicodeMath não a consegue escrever. Se nenhum dos dois a conseguir escrever, a caixa abre com o texto da equação. Clique em **OK** sem alterações, ou em **Cancelar**, para deixar a equação como está, ou escreva uma nova para a substituir.

## O separador Equação

Selecione uma equação e aparece no fim do friso um separador chamado **Equação**. Desaparece quando selecionar outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** alteram a forma como a equação selecionada é mostrada na página: Profissional desenha-a em camadas, e Linear mostra a linha em que foi escrita. A escolha fica guardada com a equação no ficheiro `.jpub`, pelo que é a mesma quando voltar a abrir o ficheiro. Um ficheiro `.pub` recebe a imagem da equação tal como é mostrada na página. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Da próxima vez que editar uma equação, ela será escrita no formato escolhido. Uma equação que nenhuma linha de tipo simples consegue escrever, como uma soma com os limites em cima e em baixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, conjuntos e lógica. Escolha um símbolo e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, índice, radical, integral, operador grande, parênteses, função, acento, limite, operador e matriz. Clique num botão para a sua forma habitual, ou na seta ao lado para mais formas, como uma derivada, uma raiz com índice ou uma matriz identidade.

Uma estrutura é inserida no fim da linha, com letras de exemplo, e a primeira fica selecionada para que possa escrever por cima. A caixa tem as mesmas estruturas numa fila de pequenos botões por cima da linha. Aí, a estrutura é inserida no cursor, e se parte da linha estiver selecionada, essa parte passa para o primeiro lugar da estrutura.

## Mover, redimensionar e colorir

Arraste a equação para a mover. Arraste um canto para a aumentar ou diminuir; a equação mantém as proporções. Use **Preenchimento da Forma** para alterar a cor das letras.

## Guardar

- **.jpub** guarda a equação, para que possa voltar a editá-la.
- **PDF e impressão** desenham a equação como formas, pelo que se mantém nítida a qualquer tamanho.
- **.pub** não tem equações. O JeffPub guarda cada uma como uma imagem, no mesmo local e com o mesmo tamanho, e avisa-o ao guardar.
- **EPUB** e **HTML** levam a equação como a linha que escreveu.
