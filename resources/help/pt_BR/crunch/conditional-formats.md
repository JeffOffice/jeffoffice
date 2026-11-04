# Formatação condicional

<!-- keywords: formatação condicional, formatar condicionalmente, regra, realçar, escala de cores, barra de dados, conjunto de ícones, ícones, primeiros, últimos, acima da média, abaixo da média, duplicado, exclusivo, fórmula, parar se verdadeiro, prioridade, barras, setas, semáforo, gradiente, formato diferencial, botão Formatação Condicional, limpar regras, maior que, menor que, entre, texto que contém, valores duplicados, top 10 -->

Uma **formatação condicional** é uma regra que muda a aparência de uma célula quando o que ela contém atende a uma condição. Exemplo: uma regra diz "pintar a célula de vermelho se ela for menor que 100". Uma célula com 80 fica vermelha, uma célula com 120 não fica, e quando você troca 80 por 150, o vermelho desaparece sozinho. A formatação da própria célula não é alterada: a regra apenas define o que é desenhado.

## O que o JeffCrunch mostra

O JeffCrunch mostra as regras que uma pasta de trabalho tem:

- **Regras de realce**: uma regra que altera a fonte, o preenchimento, a borda ou o formato de número das células que a atendem. A condição pode ser uma comparação com um valor (maior que, entre, igual a, e assim por diante), uma fórmula, texto que contém, começa com ou termina com algumas palavras, uma data como "semana passada", células em branco ou com erro, valores que estão entre os mais altos ou mais baixos (um número ou uma porcentagem), valores acima ou abaixo da média, e valores duplicados ou exclusivos.
- **Escalas de cores**: cada célula é preenchida com uma cor que passa entre duas ou três cores conforme o número vai de baixo a alto. Uma escala do branco ao verde deixa os maiores números mais verdes.
- **Barras de dados**: uma barra na célula, tão longa quanto o número é grande. As barras podem ter cor sólida ou desvanecer, podem ter borda, podem ter eixo e podem seguir para o outro lado, em outra cor, para números negativos.
- **Conjuntos de ícones**: uma pequena figura na célula que indica em que parte do intervalo o número está, como uma seta para cima, uma seta para o lado ou uma seta para baixo; ou um semáforo vermelho, amarelo ou verde. Há vinte conjuntos.

Quando duas regras dizem algo sobre a mesma célula, vence a que vem primeiro na ordem da pasta de trabalho para o que ambas definem, e uma regra marcada como "parar se verdadeiro" encerra a busca. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados com base nas células da própria regra, e são recalculados quando uma dessas células muda.

## O que continua igual

- As regras acompanham as células a que pertencem. Inserir ou excluir linhas ou colunas, recortar e colar: as regras continuam cobrindo as células certas; **Desfazer** as devolve ao lugar.
- Uma regra que o JeffCrunch não desenha é mantida no arquivo exatamente como estava.
- Ao salvar, são gravadas as regras que a pasta de trabalho tinha. Quando você cria ou limpa regras em uma planilha, as regras da planilha são gravadas novamente com as suas alterações.

## Criar uma regra

1. Selecione as células.
2. Clique em **Página Inicial > Formatação Condicional**.
3. Em **Regras de Realce de Células**, escolha **Maior Que**, **Menor Que**, **Entre**, **Igual a**, **Texto que Contém** ou **Valores Duplicados**. Em **Regras Superiores/Inferiores**, escolha **Primeiros 10 Itens**, **Últimos 10 Itens**, **Primeiros 10%**, **Últimos 10%**, **Acima da Média** ou **Abaixo da Média**.
4. Digite o valor (ou os dois valores para **Entre**, o texto para **Texto que Contém**, ou quantos itens para as regras Superiores e Inferiores). Em **Primeiros 10 Itens**, "10" é a quantidade de células que recebem o destaque. Você pode trocar por qualquer número.
5. Escolha a aparência das células, por exemplo **Preenchimento Vermelho Claro com Texto Vermelho Escuro**, e clique em **OK**.

A regra é desenhada na hora. Cada nova regra vai para o início, então, quando duas regras querem alterar a mesma coisa, a mais recente vence. Criar uma regra é uma etapa que você pode desfazer.

## Limpar regras

**Limpar Regras das Células Selecionadas** remove todas as regras que estão em qualquer uma das células selecionadas. **Limpar Regras desta Planilha** remove todas as regras da planilha. Cada uma é uma etapa que você pode desfazer. Uma escolha não faz nada quando não há regra para limpar.

## O que ainda não foi feito

- Barras de dados, escalas de cores e conjuntos de ícones podem ser exibidos, mas ainda não podem ser criados. A janela que lista, ordena e edita todas as regras (**Gerenciar Regras**) ainda não existe. Uma regra que usa fórmula ainda não pode ser criada pelo menu.
- Uma planilha que tem barras de dados de um arquivo mais novo mantém essas barras como estão, e o JeffCrunch não cria nem limpa regras nessa planilha. Faça essas alterações no programa que criou a pasta de trabalho.
- Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo mantém as suas regras quando você a salva como pasta de trabalho .xlsx, e depois você pode adicionar regras.
