# Desenhar formas e linhas
<!-- keywords: forma, linha, seta, retângulo, círculo, oval, conexão, curva, forma livre, preenchimento, contorno, sombra, 3D, editar pontos, fluxograma -->

As formas e as linhas são objetos desenhados. Pode preenchê-los com cor, dar-lhes um contorno e escrever texto dentro de uma forma.

## Desenhar uma forma

1. Clique em **Inserir > Ilustrações > Formas**. (Também em **Base > Objetos > Formas**.)
2. Escolha uma forma. Os grupos são Linhas, Retângulos, Formas Básicas, Setas de Bloco, Formas de Equação, Fluxograma, Estrelas e Faixas, Chamadas e Botões de Ação.
3. Arraste na página.

Enquanto arrasta, mantenha premida a tecla **Shift** para obter uma forma regular: uma Oval passa a círculo, e um Retângulo passa a quadrado. Mantenha premida a tecla **Ctrl** para desenhar a partir do centro para fora. Se clicar sem arrastar, obtém uma forma com 2,54 cm de largura. Depois de desenhar uma forma, o ponteiro volta ao normal. Prima **Esc** para cancelar antes de desenhar.

## Desenhar linhas e setas

Escolha **Linha**, **Seta** ou **Seta Dupla** no grupo Linhas da galeria, ou em **Formatar Forma > Linhas**. Arraste na página. Mantenha premida a tecla **Shift** para manter a linha na horizontal, na vertical ou num ângulo de 45 graus. Para alterar uma linha mais tarde, arraste a alça em qualquer uma das extremidades.

Três outras ferramentas desenham à mão:

- **Curva**: clique em pontos. A linha curva suavemente por eles.
- **Forma Livre: Forma**: clique em pontos para obter lados retos, ou mantenha o botão premido e arraste para desenhar à mão livre.
- **Forma Livre: Rabisco**: mantenha o botão premido e desenhe um só traço.

Para concluir uma curva ou uma forma livre, faça duplo clique ou prima **Enter**. Clique no primeiro ponto para a fechar numa forma preenchida.

## Unir objetos com conexões

Uma linha pode ficar presa a outros objetos. **Conexão em Cotovelo** e **Conexão Curva** desenham conexões angulares e curvas. Quando se aproxima de uma forma, de uma caixa de texto, de uma imagem ou de uma tabela, aparecem os respetivos pontos de fixação. A extremidade encaixa no meio de um lado. Quando move, redimensiona ou roda o objeto, a linha acompanha-o. Arraste a alça amarela para deslocar a curva de uma conexão em cotovelo. Arraste uma linha para longe para a soltar e separar do objeto.

## Alterar preenchimento, contorno e efeitos

Selecione a forma e abra o separador **Formatar Forma**.

- **Estilos de Forma**: a galeria oferece aspetos rápidos.
- **Preenchimento da Forma**: escolha uma cor, **Sem Preenchimento**, **Preenchimento com Imagem…** ou **Efeitos de Preenchimento…** para uma gradação, uma textura ou um padrão.
- **Contorno da Forma**: escolha uma cor. A seta ao lado define a espessura, os tracejados e as pontas das setas.
- **Efeitos da Forma**: **Sombra**, **Reflexo**, **Brilho**, **Limites Suaves**, **Bisel** e **Rotação 3D**. **Opções de Sombra…** define a cor, o desfoque, a distância e o ângulo da sombra. **Rotação 3D** inclina a forma em perspetiva.

Para ver todas as definições de uma vez, clique no pequeno botão no canto do grupo **Estilos de Forma** para abrir a janela **Formatar Forma**.

## Adicionar texto a uma forma

Faça duplo clique na forma e escreva. Também pode selecioná-la e premir **Enter**, ou clicar com o botão direito e escolher **Adicionar Texto**.

## Remodelar uma forma

- **Alterar Forma** (em **Inserir Formas**) substitui a forma por outra.
- Uma alça losangular amarela, nas formas que a têm, ajusta a forma do objeto. Num retângulo arredondado, define o grau de arredondamento dos cantos.
- **Editar Pontos** permite mover os cantos. Quadrados brancos marcam cada ponto. Arraste um ponto para o mover. **Ctrl**+clique num ponto para o eliminar, ou **Ctrl**+clique numa aresta para acrescentar um ponto. Prima **Esc** quando terminar.

## Pontos de ligação para conexões

Uma conexão fixa-se aos pontos de ligação que a forma tem (o meio de cada lado e mais alguns em certas formas). Para acrescentar um ponto próprio, selecione a forma, clique em **Editar Pontos** e **Shift**+clique no sítio onde o ponto deve ficar. Um losango azul marca-o. As conexões podem fixar-se a ele como a qualquer outro ponto de ligação, e acompanham a forma quando a move ou redimensiona: o ponto mantém a sua posição na forma (um quarto do caminho ao longo da largura continua a um quarto do caminho). **Shift**+clique num losango para o retirar; as conexões fixas a ele ficam libertadas. Uma forma pode ter até 16 pontos de ligação próprios. Cada alteração corresponde a um passo de Anular, e os pontos são mantidos quando guarda o ficheiro como ficheiro JeffPub. Um ficheiro `.pub` mantém a conexão do lado mais próximo do seu ponto de ligação.

## Unir formas e converter contornos

Selecione duas ou mais formas na mesma página e clique em **Formatar Forma > Inserir Formas > Unir Formas**. A nova forma tem o preenchimento e a linha da forma que selecionou primeiro, e fica no lugar das antigas. As formas antigas desaparecem, e também o texto que tenham: a barra de estado indica-o e, quando mais do que uma das formas tem texto, é-lhe perguntado uma só vez antes de o remover. Uma conexão presa a uma forma antiga fica presa à nova forma quando estava no meio de um dos lados da nova forma, e fica libertada no sítio onde estava nos restantes casos. Cada escolha corresponde a um passo de Anular, e as novas formas ficam selecionadas.

- **União** cria uma só forma com tudo o que as formas cobrem.
- **Combinar** mantém o que só uma das formas cobre e recorta as zonas onde se sobrepõem.
- **Subtrair** mantém a primeira forma que selecionou, sem as outras.
- **Interseção** mantém apenas o que todas as formas cobrem.
- **Fragmentar** corta as formas em todas as peças que formam entre si. Cada peça é uma forma própria.

Pode unir até 64 formas de uma só vez, e um resultado que fosse cortado em mais de 256 peças é recusado. Formas com muitas arestas que se cruzam demorariam demasiado tempo e são recusadas com uma mensagem; uma união com mais do que alguns cruzamentos é executada em segundo plano, com uma caixa onde pode clicar em **Cancelar** (as formas ficam então como estavam). Um furo numa forma continua a ser um furo, por mais uniões que se sigam. Uma forma bloqueada, dentro de um grupo ou uma obra de arte mantém-se como está, e a barra de estado indica-o. Uma forma unida é uma forma com pontos próprios, pelo que **Editar Pontos** a pode remodelar, e fica guardada no ficheiro como qualquer outra.

**Formatar Forma > Inserir Formas > Converter** altera a forma como as formas selecionadas são desenhadas:

- **Converter em Curva** dá à forma um contorno próprio, para que os seus pontos possam ser movidos (**Editar Pontos** faz o mesmo quando começa a editar).
- **Converter em Polígono** transforma cada curva do contorno em pequenas linhas retas.
- **Converter em Contorno** transforma a linha da forma numa forma própria, preenchida com a cor da linha. A forma tem de ter uma linha.
- **Separar** transforma uma forma com várias partes (como a união de formas que não se tocam) numa forma para cada parte. Um anel separa-se no seu círculo exterior e no seu furo, cada um preenchido. O texto de uma forma fica com ela: com a primeira peça de uma forma separada.

## Reencaminhar conexões

Uma conexão é uma linha cujas extremidades estão presas a formas (desenhe-a a partir do ponto de ligação de uma forma até ao de outra). Selecione uma forma, ou a conexão, e clique em **Formatar Forma > Inserir Formas > Reencaminhar Conexões**. Cada conexão presa à forma move cada extremidade presa para o ponto de ligação da respetiva forma que esteja mais próximo da outra extremidade, de modo que o trajeto seja o mais curto possível. Corresponde a um passo de Anular. Se todas as conexões já estiverem no trajeto mais curto, a barra de estado indica-o.

## Ver também

- [Mover, alinhar, agrupar e sobrepor objetos](arrange)
- [Adicionar e ajustar imagens](pictures)
- [Ícones, códigos de barras e outros objetos](insert-objects)
- [O separador Formatar Forma](tab-shape-format)
