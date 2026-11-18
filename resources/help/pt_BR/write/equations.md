# Equações
<!-- keywords: guia equação, estruturas, símbolos, converter, equação, equações, matemática, fórmula, fração, raiz quadrada, sobrescrito, subscrito, expoente, soma, integral, grego, símbolo, inserir equação, alt+=, acento, chapéu, vetor, barra, matriz, colchetes, equações predefinidas, galeria, alterar uma equação, editar uma equação -->

Uma **equação** é uma linha de matemática, como uma fração ou uma raiz quadrada. Este tópico mostra como digitar uma equação e inseri-la em um documento.

## Inserir uma equação

1. Coloque o cursor onde a equação deve ir.
2. Clique em **Inserir > Equação** ou pressione **Alt+=**.
3. Digite a equação em uma única linha. Abaixo da caixa, você vê a equação desenhada como ficará e a linha de texto que a página mostrará. Para começar com uma equação conhecida, escolha uma na lista **Equações Predefinidas** da caixa. Clique em **OK**.

A equação é salva no documento no formato padrão de matemática, para que outros programas possam abri-la.

## Como digitar

- **Frações:** digite uma barra. `a/b` é a sobre b. `(x+1)/(x-1)` trata cada grupo entre parênteses como uma parte. `\frac{1}{2}` também funciona.
- **Potências:** digite um acento circunflexo. `x^2` é x ao quadrado. `e^(i*pi)` usa o grupo entre parênteses.
- **Subscritos:** digite um sublinhado. `x_i` é x com um i pequeno embaixo. Você pode usar os dois, como em `x_i^2`.
- **Raízes:** digite `sqrt(x+1)` para uma raiz quadrada. Digite `sqrt[3]{x}` para uma raiz cúbica.
- **Somas, integrais e produtos:** digite `\sum_(i=1)^n i`, `\int_0^1 x` ou `\prod_(k=1)^n k`. A parte depois do sublinhado fica embaixo (ou na base do sinal de integral), e a parte depois do acento circunflexo fica em cima.
- **Símbolos:** digite uma barra invertida e um nome. `\alpha` é α, `\pi` é π, `\infty` é ∞, `\leq` é ≤, `\neq` é ≠, `\times` é ×, `\pm` é ± e `\to` é →. Cerca de 70 nomes funcionam.
- **Acentos:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` e `\vec{v}` colocam uma marca sobre a letra. `\overline{ab}` desenha uma linha sobre o grupo, e `\underline{ab}` desenha uma linha embaixo dele.
- **Colchetes que crescem:** digite `\left( ... \right)`, e os colchetes crescem até a altura do que está entre eles. Use qualquer colchete depois de `\left` e `\right`, ou um ponto para deixar um dos lados sem símbolo, como em `\left[ x \right.`.
- **Matrizes:** uma **matriz** é uma grade de números. Digite `\matrix{a & b \\ c & d}`. Coloque um `&` entre as células de uma linha e duas barras invertidas entre as linhas. Use `\pmatrix` para parênteses em volta da grade, `\bmatrix` para colchetes retos e `\vmatrix` para linhas retas.
- **Funções:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` e `max` mantêm seus nomes inteiros.

Na página, o JeffWrite mostra uma equação como uma linha de texto. A equação no arquivo mantém suas frações e raízes, para que outros programas a montem corretamente.

## Digitar em UnicodeMath ou LaTeX

Ao lado da caixa há uma lista de formatos de digitação: **Linha simples** (a descrita acima), **UnicodeMath** e **LaTeX**. São duas formas de escrever matemática como texto simples, que muitas pessoas já conhecem. Quando você escolhe outro formato, o que foi digitado é escrito novamente nele.

- **UnicodeMath:** `a/b` é uma fração, `x^2_i` tem uma potência e um subscrito, `√(x)` é uma raiz e `∑_(i=1)^n i` é uma soma. Coloque palavras entre aspas, como `"se x"`, e elas permanecem como texto simples em vez de letras matemáticas. `(a|b)` cria parênteses com duas partes, e `\matrix(a&b@c&d)` cria uma matriz (`&` entre células, `@` entre linhas).
- **LaTeX:** `\frac{a}{b}` é uma fração, `\sqrt[3]{x}` é uma raiz cúbica, `\sum_{i=1}^n i` é uma soma, `\text{se x}` é texto simples e `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` é uma matriz.

Os dois botões **Profissional** e **Linear** mudam a forma como a caixa mostra a sua equação. Profissional a desenha montada, como ficará. Linear mostra apenas a linha que você digitou. Se uma equação não puder ser escrita no formato escolhido, a caixa avisa e mantém o que você digitou.

## A guia Equação

Selecione uma equação (clicando nela ou movendo o cursor sobre ela com Shift), e uma guia chamada **Equação** aparece no final da faixa de opções. Ela some quando você seleciona outra coisa.

- **Ferramentas:** **Editar Equação** abre a caixa. **Profissional** e **Linear** escolhem como a caixa começa: montada ou somente como linha. O próprio documento sempre mostra uma equação como uma linha de texto. **Texto Normal** abre a caixa com um par de aspas acrescentado, para palavras que não são matemática.
- **Converter:** escolha o formato em que a caixa abre: **Linha Simples**, **UnicodeMath** ou **LaTeX**. Na próxima vez que você editar uma equação, ela será escrita no formato escolhido. Uma equação que nenhuma linha do tipo simples consegue escrever, como uma soma com seus limites em cima e embaixo, abre em UnicodeMath.
- **Símbolos:** letras gregas, operadores, relações, setas, conjuntos e lógica. Escolha um símbolo, e a caixa abre com ele acrescentado à linha.
- **Estruturas:** fração, script, radical, integral, operador grande, colchete, função, acento, limite, operador e matriz. Clique em um botão para sua forma usual, ou na seta ao lado para mais formas, como uma derivada, uma raiz com índice ou uma matriz identidade.

Uma estrutura entra no fim da linha, com letras de exemplo, e a primeira é selecionada para que você possa digitar por cima. A caixa tem as mesmas estruturas em uma fileira de pequenos botões acima da linha. Ali, a estrutura entra na posição do cursor, e, se parte da linha estiver selecionada, essa parte vai para o primeiro lugar da estrutura.

## Equações predefinidas

Clique em **Inserir > Equações Predefinidas** para ver uma lista de equações conhecidas, como a fórmula quadrática e a área de um círculo. Clique em uma para inseri-la no cursor.

## Alterar uma equação

Coloque o cursor ao lado de uma equação, ou selecione-a, e clique em **Inserir > Equação**. A caixa abre com a equação escrita em uma linha. Altere-a e clique em **OK**. Desfazer é uma única etapa.

## Equações em outros documentos

O JeffWrite mostra as equações que um documento contém e as mantém como estavam. Uma equação criada por outro programa abre em UnicodeMath, ou em LaTeX quando o UnicodeMath não consegue escrevê-la, para que você possa alterá-la. Se nenhum dos dois conseguir escrever todas as suas partes, como os sinais de agrupamento, a caixa começa com o texto da equação. Clique em **Cancelar** para manter a equação como está, ou digite uma nova para substituí-la.
