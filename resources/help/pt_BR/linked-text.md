# Fluxo de texto entre caixas de texto
<!-- keywords: vincular caixas de texto, caixas vinculadas, criar vínculo, quebrar vínculo, fluxo de texto, continuar matéria, texto em excesso, transbordamento, jarra, continua na página, continuação da página, conectar caixas de texto, encadear, cadeia, próxima caixa, caixa anterior, fluxo automático -->

Quando uma matéria é longa demais para uma caixa de texto, você pode vincular a caixa a uma segunda caixa. O restante do texto passa a fluir para a segunda caixa. Caixas vinculadas formam uma cadeia, e todo o texto de uma cadeia é uma única matéria. Edite em qualquer caixa, e as palavras voltam a fluir por toda a cadeia.

## Vincular duas caixas

A segunda caixa deve estar vazia e não pode estar vinculada. Desenhe-a primeiro com **Página Inicial > Objetos > Desenhar caixa de texto**.

1. Clique na primeira caixa para selecioná-la. Ela deve ser a última caixa da cadeia.
2. Clique em **Caixa de Texto > Vinculação > Criar vínculo**. O ponteiro se transforma em uma jarra.
3. Clique na caixa vazia. O texto em excesso é despejado nela.

Você também pode clicar no indicador vermelho de excesso **A…** no canto inferior direito de uma caixa cheia. Isso ativa a mesma jarra.

Se, em vez disso, você clicar em um espaço vazio da página, o JeffPub desenha ali uma nova caixa, do mesmo tamanho da primeira, e a vincula.

A caixa vazia pode estar em outra página. Enquanto a jarra estiver visível, clique nessa página na lista de páginas e depois clique na caixa.

Se o vínculo falhar, a barra de status informa o motivo: a caixa em que você clicou contém texto ou já está vinculada.

## Como o texto flui

O texto preenche a primeira caixa e depois a próxima caixa, na ordem em que você as vinculou. A ordem não depende da posição das caixas na página. Se uma caixa tiver colunas, as colunas são preenchidas primeiro. Se a última caixa ainda for pequena demais, o **A…** vermelho aparece e o texto que sobrou fica oculto.

## Mover-se entre caixas vinculadas

Selecione uma caixa vinculada. Uma pequena caixa com seta no canto inferior direito leva à próxima caixa. Outra, no canto superior esquerdo, leva à caixa anterior. Você também pode usar **Caixa de Texto > Vinculação > Próximo** e **Caixa de Texto > Vinculação > Anterior**. O JeffPub muda de página automaticamente.

## Quebrar um vínculo

1. Selecione a caixa imediatamente antes da quebra.
2. Clique em **Caixa de Texto > Vinculação > Quebrar**.

Todo o texto permanece na primeira parte da cadeia. As caixas depois da quebra ficam vazias, e você pode vinculá-las novamente.

## Mostrar avisos "Continua na página" e "Continuação da página"

Um aviso informa aos leitores para onde a matéria continua.

1. Clique com o botão direito do mouse em uma caixa vinculada e escolha **Formatar Objeto…**.
2. Abra a guia **Caixa de Texto**.
3. Marque **Incluir "Continua na página…"** para adicionar um aviso na parte inferior da caixa. Marque **Incluir "Continuação da página…"** para adicionar um na parte superior.

O aviso diz "(Continua na página 4)" ou "(Continuação da página 2)" em letra pequena e itálica. O JeffPub preenche o número da página. O primeiro aparece somente quando há uma caixa depois, e o segundo somente quando há uma caixa antes.

## Deixar o JeffPub adicionar páginas

Se você usar **Inserir > Texto > Inserir Arquivo** e o texto for longo demais, o JeffPub pergunta se você deseja continuá-lo em novas páginas. Escolha **Sim**. Ele adiciona páginas com uma caixa vinculada em cada uma, na mesma posição e do mesmo tamanho que a última caixa.

## Tópicos relacionados

- [Adicionar e formatar caixas de texto](text-boxes)
- [A guia Caixa de Texto](tab-text-box)
- [Adicionar, mover, excluir e nomear páginas](pages)
