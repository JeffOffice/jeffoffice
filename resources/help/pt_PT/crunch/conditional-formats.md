# Formatação condicional

<!-- keywords: formatação condicional, formatar condicionalmente, regra, realçar, escala de cores, barra de dados, conjunto de ícones, ícones, superior, inferior, acima da média, abaixo da média, duplicado, único, fórmula, parar se verdadeiro, prioridade, barras, setas, semáforos, gradação, formato diferencial, botão Formatação Condicional, limpar regras, maior que, menor que, entre, texto que contém, valores duplicados, top 10 -->

Uma **formatação condicional** é uma regra que altera o aspeto de uma célula quando o que ela contém cumpre uma condição. Exemplo: uma regra diz "pintar uma célula de vermelho se for inferior a 100". Uma célula com 80 fica vermelha, uma célula com 120 não fica, e quando altera 80 para 150 o vermelho desaparece por si. O formato da própria célula não é alterado: a regra apenas decide o que é apresentado.

## O que o JeffCrunch mostra

O JeffCrunch mostra as regras que um livro tem:

- **Regras de realce**: uma regra que altera o tipo de letra, o preenchimento, o limite ou o formato numérico das células que a cumprem. A condição pode ser uma comparação com um valor (maior que, entre, igual a, e assim por diante), uma fórmula, texto que contém, que começa por ou que termina em determinadas palavras, uma data como "semana passada", células vazias ou com erros, valores que estão entre os mais altos ou os mais baixos (um número ou uma percentagem), valores acima ou abaixo da média, e valores duplicados ou únicos.
- **Escalas de cores**: cada célula é preenchida com uma cor que passa entre duas ou três cores à medida que o número vai do valor mais baixo para o mais alto. Uma escala de branco a verde torna os números mais altos mais verdes.
- **Barras de dados**: uma barra dentro da célula, tanto mais comprida quanto maior for o número. As barras podem ser de cor sólida ou desvanecer, podem ter limite, podem ter eixo, e podem ir no sentido contrário, numa outra cor, para números negativos.
- **Conjuntos de ícones**: uma pequena imagem na célula que indica em que parte do intervalo está o número, como uma seta para cima, uma seta horizontal ou uma seta para baixo; ou um semáforo vermelho, amarelo ou verde. Existem vinte conjuntos.

Quando duas regras dizem algo sobre a mesma célula, vence a que surge primeiro na ordem do livro para aquilo que ambas definem, e uma regra marcada como "parar se verdadeiro" interrompe a procura. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados com base nas células da própria regra, e voltam a ser calculados quando uma dessas células é alterada.

## O que se mantém

- As regras acompanham as células a que pertencem. Se inserir ou eliminar linhas ou colunas, recortar e colar, as regras continuam a abranger as células certas; **Anular** repõe-nas.
- Uma regra que o JeffCrunch não apresenta é mantida no ficheiro exatamente como estava.
- Ao guardar, são gravadas as regras que o livro tinha. Quando cria ou limpa regras numa folha, as regras da folha são gravadas de novo com as suas alterações.

## Criar uma regra

1. Selecione as células.
2. Clique em **Base > Formatação Condicional**.
3. Em **Regras de Realce de Células**, escolha **Maior Que**, **Menor Que**, **Entre**, **Igual a**, **Texto que Contém** ou **Valores Duplicados**. Em **Regras Superiores/Inferiores**, escolha **10 Itens Superiores**, **10 Itens Inferiores**, **10 % Superiores**, **10 % Inferiores**, **Acima da Média** ou **Abaixo da Média**.
4. Escreva o valor (ou os dois valores para **Entre**, o texto para **Texto que Contém**, ou quantos itens para as regras Superiores e Inferiores). Em **10 Itens Superiores**, "10" é o número de células que recebem o aspeto. Pode alterá-lo para qualquer número.
5. Escolha o aspeto das células, por exemplo **Preenchimento Vermelho Claro com Texto Vermelho Escuro**, e clique em **OK**.

A regra é apresentada de imediato. Cada nova regra fica em primeiro lugar, por isso, quando duas regras querem alterar a mesma coisa, vence a mais recente. Criar uma regra é um passo que pode anular.

## Limpar regras

**Limpar Regras das Células Selecionadas** elimina todas as regras que existam em qualquer das células selecionadas. **Limpar Regras desta Folha** elimina todas as regras da folha. Cada uma é um passo que pode anular. Uma escolha não faz nada quando não existe nenhuma regra para limpar.

## O que ainda não está feito

- As barras de dados, as escalas de cores e os conjuntos de ícones podem ser apresentados, mas ainda não podem ser criados. A janela que lista, ordena e edita todas as regras (**Gerir Regras**) ainda não existe. Uma regra que utiliza uma fórmula ainda não pode ser criada a partir do menu.
- Uma folha que tenha barras de dados de um tipo de ficheiro mais recente mantém-nas como estão, e o JeffCrunch não cria nem limpa regras nessa folha. Faça essas alterações no programa que criou o livro.
- Um livro aberto a partir de um tipo de ficheiro mais antigo mantém as suas regras quando o guarda como livro .xlsx, e pode acrescentar regras depois disso.
