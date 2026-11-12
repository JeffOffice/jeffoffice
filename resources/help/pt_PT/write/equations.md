# Equações
<!-- keywords: equação, equações, matemática, fórmula, fração, raiz quadrada, expoente, índice superior, índice inferior, potência, soma, integral, grego, símbolo, inserir equação, alt+=, acento, chapéu, vetor, barra, matriz, parênteses, equações incorporadas, galeria, alterar uma equação, editar uma equação -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. Este tópico mostra como escrever uma equação e colocá-la num documento.

## Inserir uma equação

1. Coloque o cursor onde deve ficar a equação.
2. Clique em **Inserir > Equação** ou prima **Alt+=**.
3. Escreva a equação numa linha. Por baixo da caixa, vê a equação desenhada tal como ficará, e a linha de texto que a página vai mostrar. Para começar a partir de uma equação conhecida, escolha uma na lista **Equações Incorporadas** da caixa. Clique em **OK**.

A equação é guardada no documento na forma normalizada para matemática, para que outros programas a possam abrir.

## Como escrever

- **Frações:** escreva uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como um único elemento. `\frac{1}{2}` também funciona.
- **Potências:** escreva um acento circunflexo. `x^2` é x ao quadrado. `e^(i*pi)` usa o grupo entre parênteses.
- **Índices inferiores:** escreva um sublinhado. `x_i` é x com um i pequeno em baixo. Pode usar os dois, como em `x_i^2`.
- **Raízes:** escreva `sqrt(x+1)` para uma raiz quadrada. Escreva `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** escreva `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. A parte a seguir ao sublinhado fica em baixo (ou na parte inferior do sinal de integral), e a parte a seguir ao circunflexo fica em cima.
- **Símbolos:** escreva uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤, `\neq` é ≠, `\times` é ×, `\pm` é ± e `\to` é →. Funcionam cerca de 70 nomes.
- **Acentos:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` e `\vec{v}` colocam um sinal por cima da letra. `\overline{ab}` desenha uma linha por cima do grupo, e `\underline{ab}` desenha uma por baixo.
- **Parênteses que crescem:** escreva `\left( ... \right)`, e os parênteses crescem até à altura do que está entre eles. Use qualquer parêntese depois de `\left` e de `\right`, ou um ponto para omitir um dos lados, como em `\left[ x \right.`.
- **Matrizes:** uma **matriz** é uma grelha de números. Escreva `\matrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha e duas barras invertidas entre as linhas. Use `\pmatrix` para parênteses redondos à volta da grelha, `\bmatrix` para parênteses retos e `\vmatrix` para linhas retas.
- **Funções:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` e `max` mantêm o nome por inteiro.

Na página, o JeffWrite mostra uma equação como uma linha de texto. A equação no ficheiro mantém as suas frações e raízes, para que outros programas a disponham corretamente.

## Equações incorporadas

Clique em **Inserir > Equações Incorporadas** para ver uma lista de equações conhecidas, como a fórmula quadrática e a área de um círculo. Clique numa para a inserir no cursor.

## Alterar uma equação

Coloque o cursor junto de uma equação, ou selecione-a, e clique em **Inserir > Equação**. A caixa abre com a equação escrita numa linha. Altere-a e clique em **OK**. Basta um passo para a anular.

## Equações noutros documentos

O JeffWrite mostra as equações que um documento contém e mantém-nas como estavam. Pode alterar uma equação composta pelos elementos indicados acima. Uma equação com outros elementos, como marcas de agrupamento, é mantida como está, e Inserir > Equação insere uma nova em vez de a alterar.
