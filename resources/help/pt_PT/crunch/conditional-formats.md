# Formatos condicionais
<!-- keywords: formatação condicional, formato condicional, regra, realçar, escala de cores, barra de dados, conjunto de ícones, ícones, superiores, inferiores, acima da média, abaixo da média, duplicado, exclusivo, fórmula, parar se for verdadeiro, prioridade, barras, setas, semáforos, gradação, formato diferencial, botão Formatação Condicional, limpar regras, maior que, menor que, entre, texto que contém, valores duplicados, top 10 -->

Um **formato condicional** é uma regra que altera o aspeto de uma célula quando o que ela contém cumpre uma condição. Exemplo: uma regra diz "pintar uma célula de vermelho se for inferior a 100". Uma célula com 80 fica vermelha, uma célula com 120 não fica, e quando altera 80 para 150 o vermelho desaparece por si. O formato próprio da célula não é alterado: a regra apenas decide o que é desenhado.

## O que o JeffCrunch apresenta

O JeffCrunch mostra as regras que um livro tem:

- **Regras de realce**: uma regra que altera o tipo de letra, o preenchimento, o limite ou o formato numérico das células que a cumprem. A condição pode ser uma comparação com um valor (maior que, entre, igual a, e assim por diante), uma fórmula, texto que contém, começa por ou termina em algumas palavras, uma data, como "semana passada", células vazias ou com erro, valores entre os mais altos ou mais baixos (um número ou uma percentagem), valores acima ou abaixo da média, e valores duplicados ou exclusivos.
- **Escalas de cores**: cada célula é preenchida com uma cor que passa entre duas ou três cores à medida que o seu número vai de baixo para alto. Uma escala de branco para verde torna os números mais altos mais verdes.
- **Barras de dados**: uma barra na célula tanto mais comprida quanto maior for o número. As barras podem ser de cor sólida ou esbater-se, podem ter limite, podem ter eixo e podem ir no sentido contrário, numa outra cor, para números negativos.
- **Conjuntos de ícones**: uma pequena imagem na célula que indica em que parte do intervalo o número se encontra, como uma seta para cima, uma seta horizontal ou uma seta para baixo; ou um semáforo vermelho, amarelo ou verde. Existem vinte conjuntos.

Quando duas regras dizem algo sobre a mesma célula, vence a que vem primeiro na ordem do livro para aquilo que ambas definem, e uma regra marcada como "parar se for verdadeiro" termina a pesquisa. Os números de uma escala de cores, de uma barra de dados ou de um conjunto de ícones são calculados sobre as células da própria regra, e são recalculados quando uma dessas células muda.

## O que se mantém

- As regras seguem as células a que pertencem. Se inserir ou eliminar linhas ou colunas, recortar e colar, as regras continuam a abranger as células certas; **Anular** as repõe.
- Uma regra que o JeffCrunch não desenha é mantida no ficheiro exatamente como estava.
- Ao guardar, são escritas as regras que o livro tinha. Quando cria ou limpa regras numa folha, as regras dessa folha são escritas de novo com as suas alterações.

## Criar uma regra

1. Selecione as células. Pode selecionar colunas inteiras ou a folha toda; a regra é guardada para o intervalo, e não para cada célula.
2. Clique em **Base > Formatação Condicional**.
3. Em **Regras de Realce de Células**, escolha **Maior do Que**, **Menor do Que**, **Entre**, **Igual a**, **Texto que Contém**, **Uma Data que Ocorre** ou **Valores Duplicados**. Em **Regras Superiores/Inferiores**, escolha **10 Itens Superiores**, **10 Itens Inferiores**, **10% Superiores**, **10% Inferiores**, **Acima da Média** ou **Abaixo da Média**.
4. Escreva o valor (ou os dois valores para **Entre**, o texto para **Texto que Contém**, ou quantos para as regras Superiores e Inferiores), ou escolha as datas. Em **10 Itens Superiores**, "10" é o número de células que recebem o aspeto. Pode alterá-lo para qualquer número.
5. Escolha o aspeto das células, por exemplo **Preenchimento Vermelho Claro com Texto Vermelho Escuro**, e clique em **OK**. O exemplo ao lado da lista mostra o aspeto.

Para criar um aspeto próprio, escolha **Formatar Personalizada...** no fim da lista. Abre-se uma caixa com quatro separadores: **Número**, **Tipo de Letra** (o estilo, o sublinhado, o rasurado e a cor), **Limite** (apenas tipos de linha finos) e **Preenchimento** (uma cor, ou um padrão com duas cores). A caixa começa a partir do aspeto que tinha escolhido, e a parte que não altera mantém-se. Clique em **OK** e o exemplo mostra o seu aspeto.

A regra é desenhada de imediato. Cada nova regra fica em primeiro lugar, por isso, quando duas regras querem alterar a mesma coisa, a mais recente prevalece. Criar uma regra é um passo que pode anular.

## Barras de dados, escalas de cores e conjuntos de ícones

Clique em **Base > Formatação Condicional** e, depois, em **Barras de Dados**, **Escalas de Cores** ou **Conjuntos de Ícones**, e clique numa opção. A regra é aplicada de imediato às células selecionadas, sem janela para preencher, como um passo que pode anular.

- As **Barras de Dados** existem em seis cores, cada uma com preenchimento com gradação ou preenchimento sólido.
- As **Escalas de Cores** têm doze opções, com duas ou três cores. O nome indica primeiro a cor dos números mais altos.
- Os **Conjuntos de Ícones** têm vinte conjuntos: direcionais (setas e triângulos), formas (semáforos e sinais), indicadores (símbolos e bandeiras) e classificações (estrelas, quartos e caixas). Os ícones dividem o intervalo em partes iguais.

**Nova Regra**, no fim do menu, abre uma única janela para cada tipo de regra, incluindo uma regra com fórmula própria, e uma barra de dados, escala de cores ou conjunto de ícones com definições próprias.

## Gerir Regras

**Base > Formatação Condicional > Gerir Regras** lista as regras da folha na ordem em que atuam. Use **Nova Regra**, **Editar Regra** e **Eliminar Regra**; **Mover para Cima** e **Mover para Baixo** alteram a ordem; escreva outras células em **Aplica-se a**; e marque **Parar se for verdadeiro**. Editar Regra altera tudo o que a regra tem:

- para uma **barra de dados**: o que representam a barra mais curta e a mais longa (valor mais baixo ou mais alto, um número, uma percentagem, um percentil ou uma fórmula), preenchimento com gradação ou sólido, as cores, um limite, a cor das barras negativas, a posição do eixo, o sentido das barras e **Mostrar apenas a barra**;
- para uma **escala de cores**: duas ou três cores, e o que cada cor representa;
- para um **conjunto de ícones**: o estilo dos ícones, o valor em que cada ícone começa, **Inverter ordem dos ícones** e **Mostrar apenas o ícone**;
- para as restantes regras: o valor, o texto, as datas ou a fórmula, e o aspeto. Uma regra que um ficheiro trouxe com um aspeto próprio aparece com o aspeto **Formatar Personalizada**, e o exemplo mostra como é realmente. Se alterar outra coisa na regra, o seu aspeto mantém-se exatamente como o ficheiro o tinha. Escolha **Formatar Personalizada...** para alterar o aspeto.

Premir **OK** aplica todas as alterações como um único passo que pode anular. Uma regra que não alterou mantém exatamente o que o ficheiro tinha, qualquer que seja o seu lugar na lista; uma regra que altera é escrita de novo. A fórmula de uma regra é lida a partir da célula superior esquerda do seu intervalo, e uma fórmula que escreve para uma nova regra é interpretada como se estivesse escrita na célula onde está o cursor. Uma folha protegida que não permite formatar células não aceita regras.

## O que os outros tipos de ficheiro mantêm

Apenas um livro .xlsx mantém formatos condicionais. Se guardar como um tipo de ficheiro mais antigo (.xls ou .ods), estes são omitidos, e Verificar Compatibilidade indica-o, referindo as barras de dados, as escalas de cores e os conjuntos de ícones quando a folha os tem. Um ficheiro .csv não mantém formato algum. Os formatos condicionais são impressos tal como aparecem no ecrã.

## Limpar regras

**Limpar Regras das Células Selecionadas** elimina todas as regras que existem em qualquer das células selecionadas. **Limpar Regras desta Folha** elimina todas as regras da folha. Cada uma é um passo que pode anular. Uma opção não faz nada quando não há nenhuma regra a limpar.

## O que ainda não está disponível

- Uma folha que tem barras de dados no formato de um ficheiro mais recente mantém-nas tal como estão, e o JeffCrunch não cria, altera nem limpa regras nessa folha. Faça essas alterações no programa que criou o livro.
- Um livro aberto a partir de um tipo de ficheiro mais antigo mantém as suas regras ao ser guardado como livro .xlsx, e pode acrescentar regras depois disso.
- Um aspeto próprio não pode ter linha média, grossa, dupla ou inclinada, uma linha diagonal, um preenchimento com gradação, nem um tipo ou tamanho de letra: estes elementos não fazem parte daquilo que uma regra pode alterar.
