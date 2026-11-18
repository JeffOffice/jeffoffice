# Equações
<!-- keywords: separador equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, expoente, índice superior, índice inferior, soma, integral, grego, símbolo, inserir equação, alt+=, acento, chapéu, vetor, barra, matriz, parênteses, equações incorporadas, galeria, alterar uma equação, editar uma equação -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. Este tópico mostra como escrever uma equação e a colocar num documento.

## Inserir uma equação

1. Coloque o cursor onde deve ficar a equação.
2. Clique em **Inserir > Equação**, ou prima **Alt+=**.
3. Escreva a equação numa só linha. Por baixo da caixa vê a equação desenhada tal como ficará, e a linha de texto que a página vai mostrar. Para começar a partir de uma equação conhecida, escolha uma na lista **Equações Incorporadas** da caixa. Clique em **OK**.

A equação fica guardada no documento no formato padrão para matemática, para que outros programas a possam abrir.

## Como escrever

- **Frações:** escreva uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma só parte. `\frac{1}{2}` também funciona.
- **Potências:** escreva um acento circunflexo. `x^2` é x ao quadrado. `e^(i*pi)` usa o grupo entre parênteses.
- **Índices inferiores:** escreva um underscore. `x_i` é x com um i pequeno em baixo. Pode usar os dois, como em `x_i^2`.
- **Raízes:** escreva `sqrt(x+1)` para uma raiz quadrada. Escreva `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** escreva `\sum_(i=1)^n i`, `\int_0^1 x`, ou `\prod_(k=1)^n k`. A parte a seguir ao underscore fica em baixo (ou na parte inferior do sinal de integral), e a parte a seguir ao circunflexo fica em cima.
- **Símbolos:** escreva uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤, `\neq` é ≠, `\times` é ×, `\pm` é ±, e `\to` é →. Funcionam cerca de 70 nomes.
- **Acentos:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` e `\vec{v}` colocam uma marca sobre a letra. `\overline{ab}` desenha uma linha sobre o grupo, e `\underline{ab}` desenha uma por baixo.
- **Parênteses que crescem:** escreva `\left( ... \right)`, e os parênteses crescem até à altura do que está entre eles. Use qualquer parêntese depois de `\left` e `\right`, ou um ponto para deixar um dos lados de fora, como em `\left[ x \right.`.
- **Matrizes:** uma **matriz** é uma grelha de números. Escreva `\matrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha, e duas barras invertidas entre as linhas. Use `\pmatrix` para parênteses redondos à volta da grelha, `\bmatrix` para parênteses retos, e `\vmatrix` para linhas retas.
- **Funções:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` e `max` mantêm o nome inteiro.

Na página, o JeffWrite mostra uma equação como uma linha de texto. A equação no ficheiro mantém as suas frações e raízes, para que outros programas a disponham.

## Escrever em UnicodeMath ou LaTeX

Ao lado da caixa está uma lista de formatos de escrita: **Linha simples** (a que foi descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática como texto simples, que muitas pessoas já conhecem. Quando escolhe outro formato, o que escreveu é reescrito nesse formato.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um índice inferior, `√(x)` é uma raiz, e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e elas ficam como texto simples em vez de letras matemáticas. `(a|b)` cria parênteses com duas partes, e `\matrix(a&b@c&d)` cria uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples, e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** mudam a forma como a caixa mostra a sua equação. Profissional desenha-a construída, tal como ficará. Linear mostra apenas a linha que escreveu. Se uma equação não puder ser escrita no formato escolhido, a caixa avisa e mantém o que escreveu.

## O separador Equação

Selecione uma equação (clique nela, ou mova o cursor sobre ela com Shift), e aparece um separador chamado **Equação** no fim do friso. Desaparece quando seleciona outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** escolhem a forma como a caixa abre: construída ou só a linha. O próprio documento mostra sempre uma equação como uma linha de texto. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Da próxima vez que editar uma equação, ela fica escrita no formato que escolheu. Uma equação que nenhuma linha do tipo simples consegue escrever, como uma soma com os limites em cima e em baixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, e conjuntos e lógica. Escolha um símbolo, e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, índice, radical, integral, operador grande, parênteses, função, acento, limite, operador e matriz. Clique num botão para a sua forma habitual, ou na seta ao lado para mais formas, como uma derivada, uma raiz com índice, ou uma matriz identidade.

Uma estrutura entra no fim da linha, com letras de exemplo, e a primeira fica selecionada para que possa escrever por cima dela. A caixa tem as mesmas estruturas numa fila de pequenos botões por cima da linha. Aí, uma estrutura entra na posição do cursor, e se parte da linha estiver selecionada, essa parte passa para o primeiro lugar da estrutura.

## Equações incorporadas

Clique em **Inserir > Equações Incorporadas** para ver uma lista de equações conhecidas, como a fórmula quadrática e a área de um círculo. Clique numa delas para a inserir na posição do cursor.

## Alterar uma equação

Coloque o cursor junto de uma equação, ou selecione-a, e clique em **Inserir > Equação**. A caixa abre com a equação escrita numa linha. Altere-a e clique em **OK**. Basta um passo para a anular.

## Equações noutros documentos

O JeffWrite mostra as equações que um documento contém, e mantém-nas como estavam. Uma equação criada por outro programa abre em UnicodeMath, ou em LaTeX quando o UnicodeMath não a consegue escrever, para que possa alterá-la. Se nenhum dos dois conseguir escrever todas as partes, como os sinais de agrupamento, a caixa abre com o texto da equação. Clique em **Cancelar** para manter a equação como está, ou escreva uma nova para a substituir.
