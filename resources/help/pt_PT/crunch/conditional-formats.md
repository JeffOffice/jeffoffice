# Formatação condicional
<!-- keywords: formatação condicional, formato condicional, regra, realçar, destacar, escala de cores, barra de dados, conjunto de ícones, ícones, superiores, inferiores, acima da média, abaixo da média, duplicado, único, fórmula, parar se verdadeiro, prioridade, barras, setas, semáforos, gradação, formato diferencial -->

Uma **formatação condicional** é uma regra que altera o aspeto de uma célula quando o respetivo conteúdo cumpre uma condição. Exemplo: uma regra diz "colorir uma célula de vermelho se for inferior a 100". Uma célula que contém 80 fica vermelha, uma célula que contém 120 não fica e, quando altera 80 para 150, o vermelho desaparece sozinho. O formato da própria célula não é alterado: a regra apenas decide o que é desenhado.

## O que o JeffCrunch mostra

O JeffCrunch mostra as regras que um livro tem:

- **Regras de realce**: uma regra que altera o tipo de letra, o preenchimento, o limite ou o formato de número das células que a cumprem. A condição pode ser uma comparação com um valor (maior que, entre, igual a, etc.), uma fórmula, texto que contém, começa por ou termina em determinadas palavras, uma data como "semana passada", células em branco ou com um erro, valores que estão entre os maiores ou os menores (um número ou uma percentagem), valores acima ou abaixo da média e valores duplicados ou únicos.
- **Escalas de cores**: cada célula é preenchida com uma cor que varia entre duas ou três cores à medida que o número passa de baixo para alto. Uma escala de branco para verde torna os números maiores os mais verdes.
- **Barras de dados**: uma barra na célula, tanto mais longa quanto maior for o número. As barras podem ter uma cor sólida ou desvanecer, podem ter um limite, podem ter um eixo e, para números negativos, seguem no sentido contrário e noutra cor.
- **Conjuntos de ícones**: uma pequena imagem na célula que indica em que parte do intervalo se encontra o número, como uma seta para cima, uma seta lateral ou uma seta para baixo, ou uma luz vermelha, amarela ou verde. Existem vinte conjuntos.

Quando duas regras se aplicam à mesma célula, a que surge primeiro na ordem do livro prevalece no que ambas definem, e uma regra marcada com "parar se verdadeiro" termina a procura. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados sobre as células da própria regra e são novamente calculados quando uma dessas células muda.

## O que se mantém

- As regras acompanham as células a que pertencem. Se inserir ou eliminar linhas ou colunas, ou cortar e colar, as regras continuam a abranger as células certas; **Anular** repõe-nas.
- Uma regra que o JeffCrunch não desenha é mantida no ficheiro tal como estava.
- Ao guardar, são escritas as regras que o livro tinha, sem alterações.

## O que ainda não é possível

**Ainda não é possível criar, alterar ou eliminar uma regra.** O JeffCrunch não tem nenhum comando **Formatação Condicional**: mostra o que o livro já tem e mantém-no correto ao longo das suas edições. A edição de regras chegará numa versão posterior. Até lá, crie ou altere regras no programa que criou o livro.
