# Desenhar formas e linhas
<!-- keywords: forma, linha, seta, retângulo, círculo, oval, conector, curva, forma livre, preenchimento, contorno, sombra, 3D, editar pontos, fluxograma -->

Formas e linhas são objetos desenhados. Você pode preenchê-los com cor, dar-lhes contorno e digitar texto dentro de uma forma.

## Desenhar uma forma

1. Clique em **Inserir > Ilustrações > Formas**. (Também em **Página Inicial > Objetos > Formas**.)
2. Escolha uma forma. Os grupos são Linhas, Retângulos, Formas Básicas, Setas de Bloco, Formas de Equação, Fluxograma, Estrelas e Faixas, Textos Explicativos e Botões de Ação.
3. Arraste na página.

Enquanto arrasta, mantenha **Shift** pressionada para obter uma forma com proporções iguais: uma Oval vira um círculo, e um Retângulo vira um quadrado. Mantenha **Ctrl** pressionada para desenhar a partir do centro para fora. Se clicar sem arrastar, você obtém uma forma com 2,5 cm de largura. Depois de uma forma, o ponteiro volta ao normal. Pressione **Esc** para cancelar antes de desenhar.

## Desenhar linhas e setas

Escolha **Linha**, **Seta** ou **Seta Dupla** no grupo Linhas da galeria, ou em **Formatar Forma > Linhas**. Arraste na página. Mantenha **Shift** pressionada para deixar a linha na horizontal, na vertical ou em um ângulo de 45 graus. Para alterar uma linha depois, arraste a alça em qualquer uma das extremidades.

Três outras ferramentas desenham à mão livre:

- **Curva**: clique em pontos. A linha se curva suavemente entre eles.
- **Forma livre: forma**: clique em pontos para fazer lados retos, ou mantenha o botão pressionado e arraste para desenhar à mão livre.
- **Forma livre: rabisco**: mantenha o botão pressionado e desenhe um único traço.

Para concluir uma curva ou forma livre, clique duas vezes ou pressione **Enter**. Clique no primeiro ponto para fechá-la e transformá-la em uma forma preenchida.

## Unir objetos com conectores

Uma linha pode ficar presa a outros objetos. **Conector em Cotovelo** e **Conector Curvo** desenham conectores com ângulos e com curvas. Quando você desenha perto de uma forma, caixa de texto, imagem ou tabela, os pontos de conexão dela aparecem. A extremidade se encaixa no meio de um dos lados. Ao mover, redimensionar ou girar o objeto, a linha acompanha. Arraste a alça amarela para mover a curva de um conector em cotovelo. Arraste a linha sozinha para longe para desconectá-la.

## Alterar preenchimento, contorno e efeitos

Selecione a forma e abra a guia **Formatar Forma**.

- **Estilos de Forma**: a galeria oferece aparências prontas.
- **Preenchimento da Forma**: escolha uma cor, **Sem Preenchimento**, **Preenchimento com Imagem…** ou **Efeitos de Preenchimento…** para um gradiente, textura ou padrão.
- **Contorno da Forma**: escolha uma cor. A seta ao lado define espessura, traços e pontas de seta.
- **Efeitos da Forma**: **Sombra**, **Reflexo**, **Brilho**, **Bordas Suaves**, **Bisel** e **Rotação 3D**. **Opções de Sombra…** define a cor, o desfoque, a distância e o ângulo da sombra. **Rotação 3D** inclina a forma em perspectiva.

Para ver todas as configurações de uma vez, clique no pequeno botão no canto do grupo **Estilos de Forma** para abrir a janela **Formatar Forma**.

## Adicionar texto a uma forma

Clique duas vezes na forma e digite. Você também pode selecioná-la e pressionar **Enter**, ou clicar com o botão direito nela e escolher **Adicionar Texto**.

## Alterar o formato de uma forma

- **Alterar Forma** (em **Inserir Formas**) troca por outra forma.
- Uma alça amarela em losango, nas formas que a têm, ajusta o formato da forma. Em um retângulo de cantos arredondados, ela define o quanto os cantos são arredondados.
- **Editar Pontos** permite mover os cantos. Quadrados brancos marcam cada ponto. Arraste um ponto para movê-lo. Clique em um ponto com **Ctrl** pressionada para excluí-lo, ou clique em uma borda com **Ctrl** pressionada para adicionar um ponto. Pressione **Esc** ao terminar.

## Pontos de conexão para conectores

Um conector se prende aos pontos de conexão que uma forma tem (o meio de cada lado e, em algumas formas, outros pontos). Para adicionar um ponto próprio, selecione a forma, clique em **Editar Pontos** e use **Shift**+clique no local desejado. Um losango azul marca o ponto. Os conectores podem se prender a ele como a qualquer outro ponto de conexão, e acompanham a forma quando ela é movida ou redimensionada: o ponto mantém sua posição na forma (um quarto do caminho pela largura continua um quarto do caminho). Use **Shift**+clique em um losango para removê-lo; os conectores presos a ele são soltos. Uma forma pode ter até 16 pontos de conexão próprios. Cada alteração é uma etapa de **Desfazer**, e os pontos são mantidos ao salvar o arquivo como JeffPub. Um arquivo `.pub` mantém o conector do lado mais próximo do ponto de conexão.

## Mesclar formas e converter contornos

Selecione duas ou mais formas na mesma página e clique em **Formatar Forma > Inserir Formas > Mesclar Formas**. A nova forma recebe o preenchimento e a linha da forma que você selecionou primeiro, e fica no lugar das antigas. As formas originais são excluídas, e também qualquer texto nelas: a barra de status informa isso e, quando mais de uma das formas tem texto, você é perguntado uma única vez antes de o texto ser removido. Um conector preso a uma forma antiga passa a ficar preso à nova forma quando estava no meio de um dos lados dela, e é solto no ponto onde estava nos demais casos. Cada escolha é uma etapa de **Desfazer**, e as novas formas ficam selecionadas.

- **União** cria uma só forma com tudo o que as formas cobrem.
- **Combinar** mantém o que apenas uma das formas cobre e recorta as áreas em que elas se sobrepõem.
- **Subtrair** mantém a primeira forma selecionada, sem as outras.
- **Interseção** mantém apenas o que todas as formas cobrem.
- **Fragmentar** corta as formas em todas as partes que elas formam entre si. Cada parte se torna uma forma própria.

Você pode mesclar até 64 formas de uma vez, e um resultado que seria cortado em mais de 256 partes é recusado. Formas com muitas bordas que se cruzam levariam tempo demais e são recusadas com uma mensagem; uma mesclagem com mais do que poucos cruzamentos é executada em segundo plano, com uma caixa na qual você pode clicar em **Cancelar** (as formas então permanecem como estavam). Um furo em uma forma continua sendo um furo, não importa quantas mesclagens sejam feitas depois. Uma forma bloqueada, dentro de um grupo ou que seja uma obra de arte permanece como está, e a barra de status informa isso. Uma forma mesclada é uma forma com pontos próprios, portanto **Editar Pontos** a remodela, e ela é salva no arquivo como qualquer outra.

**Formatar Forma > Inserir Formas > Converter** altera a forma como as formas selecionadas são desenhadas:

- **Converter em Curva** dá à forma um contorno próprio, para que seus pontos possam ser movidos (**Editar Pontos** faz o mesmo quando você começa a editar).
- **Converter em Polígono** transforma cada curva do contorno em pequenas linhas retas.
- **Converter em Contorno** transforma a linha da forma em uma forma própria, preenchida com a cor da linha. A forma precisa ter uma linha.
- **Separar** transforma uma forma com várias partes (como a união de formas que não se tocam) em uma forma para cada parte. Um anel se separa em seu círculo externo e em seu furo, cada um preenchido. O texto de uma forma permanece com ela: com a primeira parte de uma forma separada.

## Redirecionar conectores

Um conector é uma linha cujas extremidades estão presas a formas (desenhe-a do ponto de conexão de uma forma até o de outra). Selecione uma forma ou o conector e clique em **Formatar Forma > Inserir Formas > Redirecionar Conectores**. Cada conector preso à forma move cada extremidade presa para o ponto de conexão de sua forma que estiver mais próximo da outra extremidade, de modo que o caminho seja o mais curto. Isso é uma etapa de **Desfazer**. Se todos os conectores já estiverem no caminho mais curto, a barra de status informa isso.

## Consulte também

- [Mover, alinhar, agrupar e organizar objetos em camadas](arrange)
- [Adicionar e ajustar imagens](pictures)
- [Ícones, códigos de barras e outros objetos](insert-objects)
- [A guia Formatar Forma](tab-shape-format)
