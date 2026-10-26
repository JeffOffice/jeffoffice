# Formatação condicional
<!-- keywords: formatação condicional, formato condicional, regra, realçar, destacar, escala de cores, barra de dados, barras de dados, conjunto de ícones, ícones, primeiros, últimos, acima da média, abaixo da média, duplicado, duplicatas, exclusivo, valores únicos, fórmula, parar se verdadeiro, prioridade, barras, setas, semáforo, gradiente, formato diferencial -->

Uma **formatação condicional** é uma regra que altera a aparência de uma célula quando o conteúdo dela atende a uma condição. Exemplo: uma regra diz "colorir a célula de vermelho se o valor for menor que 100". Uma célula com 80 fica vermelha, uma célula com 120 não fica, e quando você altera 80 para 150 o vermelho desaparece sozinho. O formato da própria célula não é alterado: a regra decide apenas o que é exibido.

## O que o JeffCrunch mostra

O JeffCrunch mostra as regras que uma pasta de trabalho tem:

- **Regras de realce**: uma regra que altera a fonte, o preenchimento, a borda ou o formato de número das células que a atendem. A condição pode ser uma comparação com um valor (maior que, entre, igual a e assim por diante), uma fórmula, um texto que contém, começa com ou termina com determinadas palavras, uma data como "semana passada", células em branco ou com erro, valores que estão entre os maiores ou os menores (por quantidade ou porcentagem), valores acima ou abaixo da média e valores duplicados ou exclusivos.
- **Escalas de cores**: cada célula é preenchida com uma cor que varia entre duas ou três cores conforme o número vai do menor ao maior. Uma escala do branco ao verde deixa os maiores números mais verdes.
- **Barras de dados**: uma barra na célula, tão longa quanto maior for o número. As barras podem ter cor sólida ou esmaecer, podem ter borda, podem ter um eixo e, para números negativos, seguem no sentido oposto, em outra cor.
- **Conjuntos de ícones**: uma pequena imagem na célula que indica em que parte do intervalo o número está, como uma seta para cima, uma seta para o lado ou uma seta para baixo; ou uma luz vermelha, amarela ou verde. Há vinte conjuntos.

Quando duas regras se aplicam à mesma célula, a que vem primeiro na ordem da pasta de trabalho prevalece naquilo que ambas definem, e uma regra marcada como "parar se verdadeiro" encerra a busca. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados sobre as próprias células da regra e são recalculados quando uma dessas células é alterada.

## O que permanece igual

- As regras acompanham as células às quais pertencem. Insira ou exclua linhas ou colunas, recorte e cole, e as regras continuam abrangendo as células certas; **Desfazer** as restaura.
- Uma regra que o JeffCrunch não exibe é mantida no arquivo exatamente como estava.
- Ao salvar, as regras que a pasta de trabalho tinha são gravadas sem alterações.

## O que ainda não é possível

**Ainda não é possível criar, alterar ou excluir uma regra.** O JeffCrunch não tem o comando **Formatação Condicional**: ele mostra o que a pasta de trabalho já tem e mantém tudo correto durante as suas edições. A edição de regras virá em uma versão futura. Até lá, crie ou altere as regras no programa em que a pasta de trabalho foi criada.
