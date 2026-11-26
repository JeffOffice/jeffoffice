# Formatação condicional
<!-- keywords: formatação condicional, formato condicional, regra, realçar, escala de cores, barra de dados, conjunto de ícones, ícones, superiores, inferiores, acima da média, abaixo da média, duplicado, único, fórmula, parar se for verdadeiro, prioridade, barras, setas, semáforos, gradação, formato diferencial, botão Formatação Condicional, limpar regras, maior que, menor que, entre, texto que contém, valores duplicados, 10 primeiros -->

Uma **formatação condicional** é uma regra que altera o aspeto de uma célula quando o seu conteúdo cumpre uma condição. Exemplo: uma regra diz "colorir a célula de vermelho se for inferior a 100". Uma célula com 80 fica vermelha, uma célula com 120 não fica, e quando alterar 80 para 150 o vermelho desaparece automaticamente. A formatação da própria célula não é alterada: a regra apenas decide o que é apresentado.

## O que o JeffCrunch apresenta

O JeffCrunch apresenta as regras que um livro tem:

- **Regras de realce**: uma regra que altera o tipo de letra, o preenchimento, o limite ou o formato numérico das células que cumprem a condição. A condição pode ser uma comparação com um valor (maior que, entre, igual a, e assim por diante), uma fórmula, um texto que contém, começa por ou termina em algumas palavras, uma data como "semana passada", células vazias ou com um erro, valores que estão entre os mais altos ou mais baixos (um número ou uma percentagem), valores acima ou abaixo da média, e valores duplicados ou únicos.
- **Escalas de cores**: cada célula é preenchida com uma cor que passa entre duas ou três cores à medida que o número aumenta de baixo para cima. Uma escala de branco para verde torna os números mais altos mais verdes.
- **Barras de dados**: uma barra na célula, tanto mais comprida quanto maior for o número. As barras podem ser de cor sólida ou desvanecer, podem ter limite, podem ter eixo, e podem apresentar-se em sentido contrário, numa outra cor, para números negativos.
- **Conjuntos de ícones**: uma pequena imagem na célula que indica em que parte do intervalo se encontra o número, como uma seta para cima, uma seta horizontal ou uma seta para baixo; ou um semáforo vermelho, amarelo ou verde. Existem vinte conjuntos.

Quando duas regras dizem algo sobre a mesma célula, vence a que vem primeiro na ordem do livro para aquilo que ambas definem, e uma regra marcada como "parar se for verdadeiro" termina a pesquisa. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados sobre as células da própria regra, e voltam a ser calculados quando uma dessas células é alterada.

## O que se mantém

- As regras acompanham as células a que pertencem. Se inserir ou eliminar linhas ou colunas, recortar e colar, as regras continuam a abranger as células certas; **Anular** repõe-nas.
- Uma regra que o JeffCrunch não desenha é mantida no ficheiro tal como estava.
- Ao guardar, são escritas as regras que o livro tinha. Quando cria ou limpa regras numa folha, as regras da folha são escritas de novo com as suas alterações.

## Criar uma regra

1. Selecione as células. Pode selecionar colunas inteiras ou a folha toda; a regra é guardada para o intervalo, e não para cada célula.
2. Clique em **Base > Formatação Condicional**.
3. Em **Regras de Realce de Células**, escolha **Maior Que**, **Menor Que**, **Entre**, **Igual a**, **Texto que Contém**, **Uma Data que Ocorre** ou **Valores Duplicados**. Em **Regras Superiores/Inferiores**, escolha **10 Itens Superiores**, **10 Itens Inferiores**, **10% Superiores**, **10% Inferiores**, **Acima da Média** ou **Abaixo da Média**.
4. Escreva o valor (ou os dois valores para **Entre**, o texto para **Texto que Contém**, ou quantos para as regras Superiores e Inferiores), ou escolha as datas. Em **10 Itens Superiores**, "10" é o número de células que recebem o aspeto. Pode alterá-lo para qualquer número.
5. Escolha o aspeto das células, por exemplo **Preenchimento Vermelho Claro com Texto Vermelho Escuro**, e clique em **OK**. A amostra ao lado da lista mostra o aspeto.

Para criar um aspeto próprio, escolha **Formatação Personalizada...** no fim da lista. Abre-se uma caixa com quatro separadores: **Número**, **Tipo de Letra** (o estilo, o sublinhado, o rasurado e a cor), **Limite** (apenas tipos de linha finos) e **Preenchimento** (uma cor, ou um padrão com duas cores). A caixa começa a partir do aspeto que escolheu, e uma parte que não altere mantém-se como estava. Clique em **OK** e a amostra mostra o seu aspeto.

A regra é desenhada de imediato. Cada nova regra fica em primeiro lugar, pelo que, quando duas regras querem alterar a mesma coisa, vence a mais recente. Criar uma regra é um passo que pode anular.

## Barras de dados, escalas de cores e conjuntos de ícones

Clique em **Base > Formatação Condicional**, depois em **Barras de Dados**, **Escalas de Cores** ou **Conjuntos de Ícones**, e clique numa opção. A regra é aplicada de imediato às células selecionadas, sem janela para preencher, como um passo que pode anular.

- As **Barras de Dados** existem em seis cores, cada uma com preenchimento em gradação ou preenchimento sólido.
- As **Escalas de Cores** têm doze opções, com duas cores ou três. O nome indica primeiro a cor dos números mais altos.
- Os **Conjuntos de Ícones** têm vinte conjuntos: direcionais (setas e triângulos), formas (semáforos e sinais), indicadores (símbolos e bandeiras) e classificações (estrelas, quartos e caixas). Os ícones dividem o intervalo em partes iguais.

**Nova Regra**, no fim do menu, abre uma única janela para todos os tipos de regra, incluindo uma regra com uma fórmula própria, e uma barra de dados, escala de cores ou conjunto de ícones com definições próprias.

## Gerir Regras

**Base > Formatação Condicional > Gerir Regras** lista as regras da folha pela ordem em que atuam. Use **Nova Regra**, **Editar Regra** e **Eliminar Regra**; **Mover para Cima** e **Mover para Baixo** alteram a ordem; escreva outras células em **Aplicar a**; e marque **Parar se for verdadeiro**. Editar Regra altera tudo o que uma regra tem:

- para uma **barra de dados**: o que representam as barras mais curtas e mais longas (valor mais baixo ou mais alto, um número, uma percentagem, um percentil ou uma fórmula), preenchimento em gradação ou sólido, as cores, um limite, a cor das barras negativas, onde fica o eixo, em que sentido correm as barras, e **Mostrar apenas a barra**;
- para uma **escala de cores**: duas ou três cores, e o que representa cada cor;
- para um **conjunto de ícones**: o estilo dos ícones, o valor a partir do qual começa cada ícone, **Inverter ordem dos ícones**, e **Mostrar apenas o ícone**;
- para as outras regras: o valor, o texto, as datas ou a fórmula, e o aspeto. Uma regra que um ficheiro trouxe com um aspeto próprio aparece com o aspeto **Formatação Personalizada**, e a amostra mostra o aspeto real. Se alterar outra coisa da regra, o seu aspeto mantém-se exatamente como o ficheiro o tinha. Escolha **Formatação Personalizada...** para alterar o aspeto.

Premir **OK** aplica todas as alterações como um único passo que pode anular. Uma regra que não alterou mantém exatamente o que o ficheiro tinha, qualquer que seja o seu lugar na lista; uma regra alterada é escrita de novo. A fórmula de uma regra é lida a partir da célula superior esquerda do seu intervalo, e uma fórmula escrita para uma nova regra é lida como se estivesse escrita na célula com o cursor. Uma folha protegida que não permita formatar células não aceita regras.

## O que os outros tipos de ficheiro mantêm

Apenas um livro .xlsx mantém formatações condicionais. Se guardar como um tipo de ficheiro mais antigo (.xls ou .ods), estes formatos ficam de fora, e a Verificação de Compatibilidade indica-o, referindo as barras de dados, as escalas de cores e os conjuntos de ícones quando a folha os tem. Um ficheiro .csv não mantém qualquer formato. Os formatos condicionais são impressos tal como aparecem no ecrã.

## Limpar regras

**Limpar Regras das Células Selecionadas** retira todas as regras que estejam em qualquer das células selecionadas. **Limpar Regras desta Folha** retira todas as regras da folha. **Limpar Regras desta Tabela** e **Limpar Regras desta Tabela Dinâmica** retiram as regras que estão na tabela (ou tabela dinâmica) onde se encontram as células selecionadas, e só ficam ativas quando a seleção está numa delas. Cada uma é um passo que pode anular. Uma opção não faz nada quando não há regra para limpar.

## O que ainda não está feito

- Uma folha que tenha barras de dados no formato de um ficheiro mais recente mantém-nas tal como estão, e o JeffCrunch não cria, altera nem limpa regras nessa folha. Faça essas alterações no programa que criou o livro.
- Um livro aberto a partir de um tipo de ficheiro mais antigo mantém as suas regras quando o guarda como livro .xlsx, e pode acrescentar regras depois disso.
- Um aspeto próprio não pode ter linha média, grossa, dupla ou inclinada, uma linha diagonal, um preenchimento em gradação, nem um tipo ou tamanho de letra: estes elementos não fazem parte do que uma regra pode alterar.
