# Formatação condicional
<!-- keywords: formatação condicional, formatar condicionalmente, regra, realçar, destaque, escala de cores, barra de dados, conjunto de ícones, ícones, maiores, menores, acima da média, abaixo da média, duplicado, duplicata, exclusivo, único, fórmula, parar se verdadeiro, prioridade, barras, setas, semáforos, degradê, regra de formatação diferencial, botão Formatação Condicional, limpar regras, maior que, menor que, entre, texto que contém, valores duplicados, 10 primeiros, top 10 -->

Uma **formatação condicional** é uma regra que muda a aparência de uma célula quando o conteúdo dela atende a uma condição. Exemplo: uma regra diz "pintar a célula de vermelho se for menor que 100". Uma célula com 80 fica vermelha, uma célula com 120 não fica, e quando você troca 80 por 150, o vermelho desaparece sozinho. A formatação própria da célula não é alterada: a regra apenas decide o que é desenhado.

## O que o JeffCrunch mostra

O JeffCrunch mostra as regras que uma pasta de trabalho tem:

- **Regras de realce**: uma regra que altera a fonte, o preenchimento, a borda ou o formato numérico das células que atendem a ela. A condição pode ser uma comparação com um valor (maior que, entre, igual a, e assim por diante), uma fórmula, um texto que contém, que começa ou que termina com algumas palavras, uma data como "semana passada", células em branco ou com erro, valores que estão entre os mais altos ou os mais baixos (um número ou uma porcentagem), valores acima ou abaixo da média, e valores duplicados ou exclusivos.
- **Escalas de cores**: cada célula é preenchida com uma cor que varia entre duas ou três cores conforme o número vai do menor para o maior. Uma escala do branco ao verde deixa os maiores números mais verdes.
- **Barras de dados**: uma barra na célula, tão longa quanto o número é grande. As barras podem ser de cor sólida ou em degradê, podem ter borda, podem ter eixo e podem seguir para o outro lado, em outra cor, para números negativos.
- **Conjuntos de ícones**: uma pequena figura na célula que indica em qual parte do intervalo o número se encontra, como uma seta para cima, uma seta horizontal ou uma seta para baixo; ou um semáforo vermelho, amarelo ou verde. Há vinte conjuntos.

Quando duas regras afetam a mesma célula, vale a que vem primeiro na ordem da pasta de trabalho para o que ambas definem, e uma regra marcada como "parar se verdadeiro" encerra a busca. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados sobre as células da própria regra, e são recalculados quando uma dessas células muda.

## O que permanece igual

- As regras acompanham as células a que pertencem. Inserir ou excluir linhas ou colunas, recortar e colar: as regras continuam cobrindo as células certas; **Desfazer** as restaura.
- Uma regra que o JeffCrunch não desenha é mantida no arquivo exatamente como estava.
- Ao salvar, são gravadas as regras que a pasta de trabalho tinha. Quando você cria ou limpa regras em uma planilha, as regras dela são gravadas novamente com as suas alterações.

## Criar uma regra

1. Selecione as células. Você pode selecionar colunas inteiras ou a planilha toda; a regra é mantida para o intervalo, e não para cada célula.
2. Clique em **Página Inicial > Formatação Condicional**.
3. Em **Regras de Realce de Células**, escolha **Maior Que**, **Menor Que**, **Entre**, **Igual a**, **Texto que Contém**, **Uma Data que Ocorre** ou **Valores Duplicados**. Em **Regras de Valores Superiores e Inferiores**, escolha **Primeiros 10 Itens**, **Últimos 10 Itens**, **Primeiros 10%**, **Últimos 10%**, **Acima da Média** ou **Abaixo da Média**.
4. Digite o valor (ou os dois valores para **Entre**, o texto para **Texto que Contém**, ou quantos itens para as regras de valores superiores e inferiores), ou escolha as datas. Para **Primeiros 10 Itens**, "10" é quantas células recebem o visual. Você pode alterá-lo para qualquer número.
5. Escolha como as células devem ficar, por exemplo **Preenchimento Vermelho Claro com Texto Vermelho Escuro**, e clique em **OK**. A amostra ao lado da lista mostra a aparência.

Para criar uma aparência própria, escolha **Formato Personalizado...** no fim da lista. Abre-se uma caixa com quatro guias: **Número**, **Fonte** (o estilo, o sublinhado, o tachado e a cor), **Borda** (apenas tipos de linha finos) e **Preenchimento** (uma cor, ou um padrão com duas cores). A caixa começa com a aparência que você havia escolhido, e uma parte que você não altera permanece como estava. Clique em **OK** e a amostra mostra a sua aparência.

A regra é desenhada imediatamente. Cada nova regra vai para o início, então, quando duas regras querem alterar a mesma coisa, a mais nova vence. Criar uma regra é uma etapa que você pode desfazer.

## Barras de dados, escalas de cores e conjuntos de ícones

Clique em **Página Inicial > Formatação Condicional**, depois em **Barras de Dados**, **Escalas de Cores** ou **Conjuntos de Ícones**, e clique em uma opção. A regra é aplicada às células selecionadas imediatamente, sem janela para preencher, como uma etapa que você pode desfazer.

- **Barras de Dados** têm seis cores, cada uma com preenchimento em degradê ou com preenchimento sólido.
- **Escalas de Cores** tem doze opções, com duas cores ou três. O nome indica primeiro a cor dos números mais altos.
- **Conjuntos de Ícones** tem vinte conjuntos: direcionais (setas e triângulos), formas (semáforos e placas), indicadores (símbolos e bandeiras) e classificações (estrelas, quartos e caixas). Os ícones dividem o intervalo em faixas de mesmo tamanho.

**Nova Regra**, no fim do menu, abre uma única janela para todos os tipos de regra, incluindo uma regra com fórmula própria, e uma barra de dados, escala de cores ou conjunto de ícones com configurações personalizadas.

## Gerenciar Regras

**Página Inicial > Formatação Condicional > Gerenciar Regras** lista as regras da planilha na ordem em que funcionam. Use **Nova Regra**, **Editar Regra** e **Excluir Regra**; **Mover para Cima** e **Mover para Baixo** alteram a ordem; digite outras células em **Aplicar a**; e marque **Parar se verdadeiro**. Editar Regra altera tudo o que a regra tem:

- para uma **barra de dados**: o que representam as barras mais curtas e mais longas (valor mais baixo ou mais alto, um número, uma porcentagem, um percentil ou uma fórmula), preenchimento em degradê ou sólido, as cores, uma borda, a cor das barras negativas, a posição do eixo, o sentido das barras e **Mostrar somente a barra**;
- para uma **escala de cores**: duas ou três cores, e o que cada cor representa;
- para um **conjunto de ícones**: o estilo dos ícones, o valor em que cada ícone começa, **Inverter ordem dos ícones** e **Mostrar somente o ícone**;
- para as demais regras: o valor, o texto, as datas ou a fórmula, e a aparência. Uma regra que um arquivo trouxe com aparência própria aparece listada com a aparência **Formato Personalizado**, e a amostra mostra como ela realmente fica. Se você alterar outra coisa na regra, a aparência dela permanece exatamente como estava no arquivo. Escolha **Formato Personalizado...** para alterar a aparência.

Clicar em **OK** aplica todas as suas alterações como uma única etapa que você pode desfazer. Uma regra que você não alterou mantém exatamente o que o arquivo tinha, qualquer que seja a sua posição na lista; uma regra alterada é gravada novamente. A fórmula de uma regra é lida a partir da célula superior esquerda do intervalo dela, e uma fórmula digitada para uma nova regra é lida como se estivesse escrita na célula com o cursor. Uma planilha protegida que não permite formatar células não recebe regras.

## O que outros tipos de arquivo mantêm

Apenas uma pasta de trabalho .xlsx mantém formatações condicionais. Salvar em um tipo de arquivo mais antigo (.xls ou .ods) deixa-as de fora, e Verificar Compatibilidade informa isso, citando barras de dados, escalas de cores e conjuntos de ícones quando a planilha os tem. Um arquivo .csv não mantém formatação alguma. As formatações condicionais são impressas como aparecem na tela.

## Limpar regras

**Limpar Regras das Células Selecionadas** remove todas as regras que estão em qualquer uma das células selecionadas. **Limpar Regras desta Planilha** remove todas as regras da planilha. **Limpar Regras Desta Tabela** e **Limpar Regras Desta Tabela Dinâmica** removem as regras que estão na tabela (ou tabela dinâmica) em que as células selecionadas se encontram, e ficam disponíveis apenas quando a seleção está em uma delas. Cada uma é uma etapa que você pode desfazer. Uma opção não faz nada quando não há regra a limpar.

## O que ainda não é feito

- Uma planilha que tem barras de dados no formato de um arquivo mais recente as mantém como estão, e o JeffCrunch não cria, altera nem limpa regras nessa planilha. Faça essas alterações no programa que criou a pasta de trabalho.
- Uma pasta de trabalho aberta a partir de um tipo de arquivo mais antigo mantém as suas regras quando você a salva como pasta de trabalho .xlsx, e depois você pode adicionar regras.
- Uma aparência própria não pode ter linha média, grossa, dupla ou inclinada, linha diagonal, preenchimento em degradê, nem tipo ou tamanho de fonte: esses itens não fazem parte do que uma regra pode alterar.
