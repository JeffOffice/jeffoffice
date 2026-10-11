# Marcas, numeração e tabulações
<!-- keywords: lista com marcas, lista de marcas, lista numerada, numeração, marcas, marcadores, lista, posições de tabulação, tabulações, tabuladores, régua, carácter de preenchimento, pontos de preenchimento, tabulação decimal, avanço pendente -->

Uma lista coloca cada item na sua própria linha, assinalado com uma marca ou um número. As posições de tabulação alinham o texto em colunas organizadas, como os preços numa ementa.

## Criar uma lista

1. Clique num parágrafo ou selecione vários.
2. Clique em **Base > Parágrafo > Marcas** ou **Base > Parágrafo > Numeração**.

Clique novamente no mesmo botão para desativar a lista. Cada botão tem também uma seta. A seta junto a **Marcas** oferece oito símbolos de marca. A seta junto a **Numeração** oferece sete estilos, como 1. 2. 3., a. b. c., I. II. III. e (1) (2) (3). Escolha **Nenhum** para remover uma lista.

Para mais opções, escolha **Marcas e numeração…** em qualquer um dos menus. O separador **Marcas** tem 12 símbolos e um botão **Carácter…** para escolher o seu. O separador **Numeração** tem uma caixa **Começar em**, para que uma lista possa começar em 5, por exemplo.

**Aumentar Avanço** e **Diminuir Avanço** deslocam os itens da lista um nível para dentro ou para fora, 0,6 cm de cada vez. Um item deslocado para dentro inicia uma lista própria por baixo da anterior: aí, os números passam a a. b. c. (e i. ii. iii. um nível mais abaixo) e as marcas passam a círculos (e depois quadrados). Por exemplo, selecione os itens 2 e 3 de uma lista numerada e clique em **Aumentar Avanço**: passam a a. e b. por baixo de 1., e o item seguinte continua a ser 2.

## Escrever uma lista

- Escreva `*` ou `-` e um espaço no início de um parágrafo para iniciar uma lista com marcas.
- Escreva `1.` ou `1)` e um espaço para iniciar uma lista numerada.
- Prima Enter para passar ao item seguinte. Prima Enter num item vazio para terminar a lista.
- Prima Retrocesso no início de um item para remover a respetiva marca.

Para desativar esta funcionalidade, desmarque **Formatação automática ao escrever** no separador **Verificação** de **Ficheiro > Opções** (a janela **Opções do JeffPub**).

## O que são posições de tabulação

Uma posição de tabulação é um ponto da linha para onde o texto salta quando prime a tecla Tab. Se não definir nenhuma, a tecla Tab salta para a posição de tabulação predefinida seguinte. Estas estão afastadas 1,27 cm entre si, contando a partir da margem esquerda do texto, a menos que as altere (consulte **Alterar as posições de tabulação predefinidas** abaixo). Se o texto a seguir a uma tabulação não couber na linha, a tabulação e esse texto passam juntos para o início da linha seguinte, desde que a primeira palavra caiba aí depois da posição da tabulação. Se não couber, a tabulação fica no fim da linha e o texto começa a linha seguinte na margem esquerda do parágrafo. Uma posição pode ser de quatro tipos:

- **Esquerda:** o texto começa na posição.
- **Centro:** o texto fica centrado na posição.
- **Direita:** o texto termina na posição.
- **Decimal:** as vírgulas decimais alinham-se na posição.

Uma posição pode ter também um carácter de preenchimento. Trata-se de uma fila de pontos, traços, uma linha ou marcas que preenche o espaço antes da posição. Uma linha de ementa como "Sopa ....... 4,50 €" é uma tabulação à direita com pontos de preenchimento.

As posições de tabulação pertencem aos parágrafos. Contam a partir da margem esquerda do texto, dentro da margem da caixa. As posições de tabulação predefinidas são a única exceção: pertencem a toda a publicação.

## Definir posições de tabulação na régua

1. Clique em **Ver > Mostrar > Réguas** se as réguas estiverem ocultas.
2. Clique no texto. A régua mostra a largura do texto a branco.
3. Clique na régua onde pretende uma posição. Aparece uma posição de tabulação à esquerda.
4. Arraste uma posição para a mover. Arraste-a para baixo, para fora da régua, para a remover.

## Definir posições de tabulação numa caixa de diálogo

1. Clique no parágrafo e faça duplo clique na régua. A caixa de diálogo **Parágrafo** abre no separador **Tabulações**. (Também pode abri-la a partir de **Base > Parágrafo > Espaçamento entre Linhas > Parágrafo…** e clicar no separador **Tabulações**.)
2. Clique em **Adicionar** e, em seguida, faça duplo clique na nova **Posição** para escrever um número.
3. Escolha um **Alinhamento** (esquerda, centro, direita ou decimal) e um **Carácter de preenchimento** para a linha.
4. Clique em **OK**.

**Eliminar** remove a linha selecionada. **Limpar tudo** remove todas as posições.

Numa célula de tabela, a tecla Tab passa para a célula seguinte. Consulte [Criar e editar tabelas](tables).

## Alterar as posições de tabulação predefinidas

Quando um parágrafo não tem posições de tabulação próprias, a tecla Tab salta para a posição de tabulação predefinida seguinte. Inicialmente, estão afastadas 1,27 cm entre si, e uma única definição determina o espaçamento para todos os parágrafos da publicação.

1. Abra a caixa de diálogo **Parágrafo** e clique no separador **Tabulações**, como descrito acima. Não precisa de selecionar texto primeiro.
2. Por baixo da lista de posições, escreva uma distância em **Posições de tabulação predefinidas**. Pode ser qualquer valor entre 1 ponto e 1584 pontos (55,88 cm). A caixa utiliza as unidades de medida que escolheu em **Ficheiro > Opções**, e pode escrever uma unidade, como 1 cm.
3. Clique em **OK**.

Todo o texto que utiliza as posições predefinidas passa de imediato para o novo espaçamento, e **Anular** repõe o espaçamento anterior. A publicação mantém a definição quando a guarda, como ficheiro JeffPub ou como ficheiro .pub. As posições que definir manualmente, na régua ou com **Adicionar**, ficam onde as colocou.

## Tópicos relacionados

- [Formatar texto](formatting-text)
- [Utilizar estilos](styles)
- [O separador Base](tab-home)
