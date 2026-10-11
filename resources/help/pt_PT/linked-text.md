# Distribuir texto entre caixas de texto
<!-- keywords: ligar caixas de texto, caixas ligadas, criar ligação, quebrar ligação, fluxo de texto, continuar artigo, texto em excesso, jarro, continua na página, continuação da página, unir caixas de texto, encadear, cadeia, caixa seguinte, caixa anterior, fluxo automático -->

Quando um artigo é demasiado longo para uma caixa de texto, pode ligar a caixa a uma segunda caixa. O resto do texto passa então para a segunda caixa. As caixas ligadas formam uma cadeia e todo o texto de uma cadeia é um único artigo. Se editar o texto em qualquer caixa, as palavras voltam a distribuir-se por toda a cadeia.

## Ligar duas caixas

A segunda caixa tem de estar vazia e não pode estar já ligada. Desenhe-a primeiro com **Base > Objetos > Desenhar caixa de texto**.

1. Clique na primeira caixa para a selecionar. Tem de ser a última caixa da respetiva cadeia.
2. Clique em **Caixa de Texto > Ligação > Criar ligação**. O ponteiro transforma-se num jarro.
3. Clique na caixa vazia. O texto em excesso é vertido para ela.

Também pode clicar no indicador de texto em excesso **A…** a vermelho, no canto inferior direito de uma caixa cheia. Isso ativa o mesmo jarro.

Se, em vez disso, clicar numa área vazia da página, o JeffPub desenha aí uma nova caixa, com o mesmo tamanho da primeira, e liga-a.

A caixa vazia pode estar noutra página. Enquanto o jarro estiver visível, clique nessa página na lista de páginas e, em seguida, clique na caixa.

Se a ligação falhar, a barra de estado indica o motivo: ou a caixa em que clicou contém texto, ou já está ligada.

## Como o texto é distribuído

O texto preenche a primeira caixa e, em seguida, a caixa seguinte, pela ordem em que as ligou. A ordem não depende da posição das caixas na página. Se uma caixa tiver colunas, as colunas são preenchidas primeiro. Se a última caixa continuar a ser demasiado pequena, aparece o **A…** a vermelho e o texto restante fica oculto.

## Mover-se entre caixas ligadas

Selecione uma caixa ligada. Uma pequena caixa com uma seta no canto inferior direito leva-o para a caixa seguinte. Outra no canto superior esquerdo leva-o para a caixa anterior. Também pode utilizar **Caixa de Texto > Ligação > Seguinte** e **Caixa de Texto > Ligação > Anterior**. O JeffPub muda de página automaticamente.

## Quebrar uma ligação

1. Selecione a caixa imediatamente antes da quebra.
2. Clique em **Caixa de Texto > Ligação > Quebrar**.

Todo o texto fica na primeira parte da cadeia. As caixas a seguir à quebra ficam vazias e pode voltar a ligá-las.

## Mostrar os avisos "Continua na página" e "Continuação da página"

Um aviso indica aos leitores onde o artigo continua.

1. Clique com o botão direito do rato numa caixa ligada e escolha **Formatar Objeto…**.
2. Abra o separador **Caixa de Texto**.
3. Selecione **Incluir "Continua na página…"** para adicionar um aviso na parte inferior da caixa. Selecione **Incluir "Continuação da página…"** para adicionar um na parte superior.

O aviso apresenta "(Continua na página 4)" ou "(Continuação da página 2)" em letra itálica pequena. O JeffPub preenche o número de página. O primeiro só aparece quando existe uma caixa a seguir e o segundo só quando existe uma caixa antes.

## Deixar o JeffPub adicionar páginas

Se utilizar **Inserir > Texto > Inserir Ficheiro** e o texto for demasiado longo, o JeffPub pergunta se pretende continuá-lo em páginas novas. Escolha **Sim**. São adicionadas páginas com uma caixa ligada em cada uma, na mesma posição e com o mesmo tamanho da última caixa.

## Tópicos relacionados

- [Adicionar e formatar caixas de texto](text-boxes)
- [O separador Caixa de Texto](tab-text-box)
- [Adicionar, mover, eliminar e mudar o nome de páginas](pages)
