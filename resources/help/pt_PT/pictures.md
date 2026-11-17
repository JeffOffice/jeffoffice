# Adicionar e ajustar imagens
<!-- keywords: efeitos de imagem, sombra, reflexo, brilho, limites suaves, bisel, rotação 3D, repor imagem e tamanho, foto, fotografia, imagem, inserir imagem, recortar, imagem de arquivo, página PDF, jpg, png, svg, recolorir, texto alternativo, legenda, marcador de posição, gestor de gráficos, imagens online, ligar a ficheiro, imagem ligada, inserir e ligar, incorporar, imagem em falta, religar -->

Uma imagem fica dentro de uma moldura. Pode recortar a imagem e mover ou redimensionar a moldura.

## Inserir uma imagem a partir de um ficheiro

1. Clique em **Inserir > Ilustrações > Imagens**.
2. Na janela **Inserir Imagem**, escolha um ficheiro. O JeffPub abre ficheiros PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Escolha **Inserir**, **Ligar ao Ficheiro** ou **Inserir e Ligar** na caixa **Inserir como**, que fica junto à lista de ficheiros e pode ser alcançada com a tecla Tab. **Inserir** é a opção escolhida sempre que a janela abre. Consulte "Ligar uma imagem ao seu ficheiro", mais abaixo.
4. Clique em **Abrir**.

A imagem fica no centro da página. Também pode arrastar um ficheiro de imagem para a página, o que usa sempre **Inserir**.

O JeffPub lê ficheiros de imagem até 256 MB. Um ficheiro maior é recusado, o mesmo acontecendo com um ficheiro vazio ou algo que não seja um ficheiro normal no disco, como um dispositivo ou um tubo. A mensagem indica que o JeffPub não consegue lê-lo como imagem.

Se escolher várias imagens de uma só vez, elas ficam alinhadas como miniaturas na área de rascunho, o espaço cinzento ao lado da página. Arraste para a página as que quiser.

Se um PDF tiver mais do que uma página, a janela **Inserir Página PDF** mostra cada página. Escolha uma e clique em **OK**.

## Ligar uma imagem ao seu ficheiro

**Inserir** coloca uma cópia da imagem dentro da publicação. Essa cópia segue a publicação para onde quer que ela vá, mas uma publicação com muitas fotografias grandes também fica grande. Uma ligação mantém as fotografias nos respetivos ficheiros.

- **Inserir** copia a imagem para dentro da publicação. É a escolha habitual.
- **Ligar ao Ficheiro** guarda apenas a localização do ficheiro e uma pequena pré-visualização, com no máximo 512 píxeis no lado mais comprido. A imagem na página, e na impressão, num PDF ou num livro eletrónico, vem do ficheiro com qualidade total. Se o ficheiro for movido ou eliminado, a página mostra a pequena pré-visualização.
- **Inserir e Ligar** faz as duas coisas. A publicação guarda a imagem completa e a ligação. Quando abre a publicação e o ficheiro foi alterado, o JeffPub lê-o novamente e substitui a sua cópia.

O JeffPub memoriza a localização completa do ficheiro, e também a sua posição relativamente à publicação, como "a pasta Fotos ao lado dela". Se mover a publicação e as respetivas imagens em conjunto, a segunda indicação prevalece, para que as imagens sejam encontradas novamente. Mantenha as pastas como estavam.

O JeffPub só abre por si o ficheiro de uma imagem ligada quando o ficheiro está na pasta da publicação ou numa pasta dentro dela. Uma imagem ligada a um ficheiro situado noutro local mostra a pré-visualização guardada, desfocada, e fica marcada como **Não atualizada**, até a selecionar e clicar em **Atualizar Ligação** no Gestor de Gráficos. Isto evita que uma publicação que alguém lhe envie aceda ao seu computador. Para que as imagens carreguem automaticamente, guarde-as na pasta da publicação.

O **Gestor de Gráficos** mostra se cada imagem está **Incorporada**, **Ligada**, **Em falta**, **Modificada** ou **Não atualizada**, e tem botões para atualizar uma ligação, apontá-la para outro ficheiro ou transformá-la numa cópia. Consulte [Gestor de Gráficos](graphics-manager).

Ao guardar uma publicação como ficheiro `.pub`, criar um ficheiro Pack and Go ou enviá-la por correio eletrónico, o JeffPub inclui cada imagem ligada completa no ficheiro, para que não dependa de ficheiros que ficam para trás. Se um ficheiro ligado estiver em falta nessa altura, ou se a imagem estiver **Não atualizada**, é incluída a respetiva pequena pré-visualização. Um ficheiro `.pub` que abra nunca tem ligações: as imagens entram como cópias.

## Adicionar um marcador de posição de imagem

Um marcador de posição é uma moldura vazia que reserva um lugar para uma imagem. Clique em **Inserir > Ilustrações > Marcador de Posição de Imagem**, depois faça duplo clique na moldura e escolha um ficheiro. A imagem preenche a moldura, e as arestas que sobressaem ficam ocultas. Um marcador de posição vazio não é impresso.

## Encontrar imagens gratuitas online

Clique em **Inserir > Ilustrações > Imagens online**. O painel **Imagens Online** pesquisa duas bibliotecas gratuitas de imagens com licença aberta: o Openverse, que abrange muitas coleções, e o Wikimedia Commons, a biblioteca das imagens da Wikipédia.

1. Escolha uma biblioteca, escreva o que procura, por exemplo "celeiro vermelho", e prima Enter.
2. Clique numa imagem para ver quem a criou e a respetiva licença. Uma licença é o conjunto de regras para utilizar a imagem. A maioria pede que indique o nome do autor.
3. Clique em **Inserir**. O JeffPub coloca a imagem na página e usa o respetivo título como texto alternativo. Com a opção **Adicionar um crédito por baixo da imagem** marcada, uma linha como "Celeiro vermelho" de Pat Lee, CC BY 2.0 fica por baixo dela.

As palavras da pesquisa são enviadas para a biblioteca que escolheu. Verifique se a licença de cada imagem se adequa à forma como a vai utilizar.

Uma pesquisa mostra até 24 imagens. O JeffPub utiliza apenas endereços Web seguros (https) de uma biblioteca e deixa de fora uma imagem cujo endereço não seja seguro. Do Wikimedia Commons, uma imagem com mais de 1920 píxeis de largura é obtida numa cópia com essa largura, e não no ficheiro original carregado. Um descarregamento superior a 50 MB, ou que demore demasiado, é interrompido, e o painel indica que a imagem não pôde ser descarregada nem carregada.

## Recortar uma imagem

Recortar oculta as margens de uma imagem.

1. Clique na imagem e depois em **Formato da Imagem > Recortar > Recortar**.
2. Arraste uma alça para ocultar parte da imagem. Arraste a imagem para a deslocar dentro da moldura.
3. Clique novamente em **Recortar**, ou prima **Esc**.

Também no grupo **Recortar**: **Proporção** corta a moldura a uma forma como um quadrado, 4 de largura por 3 de altura, ou 16 por 9, em torno do centro (a imagem mantém-se onde está na página); **Ajustar** mostra a imagem inteira dentro da moldura; **Preencher** preenche a moldura; **Recortar para Forma** recorta segundo uma forma, como uma elipse; e **Limpar Recorte** volta a mostrar a imagem inteira.

## Alterar o aspeto de uma imagem

No separador **Formato da Imagem**, use o grupo **Ajustar**:

- **Correções**: escolha um par de luminosidade e contraste, entre menos 40 e mais 40 por cento. **Formatar Objeto…**, no fim do menu, tem controlos deslizantes.
- **Recolorir**: torne a imagem em Escala de Cinzentos, Sépia, Desvanecido, Preto e Branco, ou com uma tonalidade de destaque. **Definir Cor Transparente** torna transparente a cor no canto superior esquerdo da imagem.
- **Transparência…**: esbate a imagem inteira.
- **Repor Imagem**: anula todas as alterações desta lista, bem como o limite, a forma e os efeitos da imagem. O recorte também é anulado, pelo que a moldura mostra a imagem inteira. Clique na seta ao lado para **Repor Imagem e Tamanho**, que também devolve à imagem o tamanho que tem no respetivo ficheiro.
- **Comprimir Imagens…**: reduz o ficheiro diminuindo o detalhe para 300, 220 ou 96 ppp (píxeis por polegada). Por predefinição, altera todas as imagens e elimina as áreas recortadas.
- **Converter em Formas**: transforma uma imagem SVG (um desenho feito de linhas e formas) em formas editáveis, recortadas onde o desenho as corta. O texto e as imagens que existam dentro dela ficam de fora.

## Estilos, limites e efeitos de imagem

Use o grupo **Estilos de Imagem** no separador **Formato da Imagem**:

- A **galeria** mostra cerca de duas dezenas de aspetos prontos: molduras, sombras, reflexos, limites suaves, formas arredondadas e ovais, uma rotação no espaço, um brilho e biséis. Um bisel é uma aresta inclinada que faz a imagem parecer elevada. Cada miniatura mostra as suas cores. Clique numa para dar à imagem esse aspeto. Um estilo define a forma, o limite e os efeitos em conjunto, e substitui os que a imagem tinha. O recorte, o tamanho e as cores mantêm-se.
- **Limite da Imagem** define a cor, a **Espessura** (a grossura da linha) e os **Tracejados** (uma linha contínua, pontilhada ou tracejada). **Sem Contorno** remove o limite.
- **Efeitos de Imagem** abre **Sombra**, **Reflexo**, **Brilho**, **Limites Suaves**, **Bisel** e **Rotação 3D**. Cada um tem opções prontas. Escolha uma para a adicionar. Os restantes efeitos mantêm-se. Escolha a primeira entrada, como **Sem Sombra**, para remover um efeito. **Mais Opções de Efeitos…** abre **Formatar Objeto**.

Cada uma destas ações pode ser desfeita com um único passo.

## Legenda, texto alternativo e troca

- **Formato da Imagem > Estilos de Imagem > Legenda** adiciona uma caixa de texto por baixo da imagem.
- O texto alternativo é uma descrição curta para pessoas que não podem ver a imagem, como os utilizadores de leitores de ecrã. Clique com o botão direito na imagem, escolha **Formatar Objeto…**, abra o separador **Texto Alternativo** e escreva em **Texto alternativo**.
- **Formato da Imagem > Inserir > Alterar Imagem** coloca outro ficheiro na mesma moldura. A moldura mantém o tamanho, o lugar e o aspeto: o limite, a forma, os efeitos e as alterações de cor. Se a nova imagem tiver a mesma forma da anterior, o recorte também se mantém. Se tiver outra forma, preenche a moldura.
- Para trocar duas imagens, arraste uma sobre a outra. Quando a dica indicar **Solte para trocar as imagens**, largue: as imagens trocam de moldura, e a moldura que arrastou volta ao seu lugar. Ou mantenha Shift premido, clique nas duas e clique em **Trocar**. Mantenha Ctrl premido ao arrastar para copiar uma imagem. Uma cópia largada sobre outra imagem fica onde a colocou, e nada é trocado.

## Ver todas as imagens

Clique em **Ver > Mostrar > Gestor de Gráficos** para listar todas as imagens no painel **Gestor de Gráficos**. Selecione uma e clique em **Ir para** para lhe ir diretamente, em **Guardar Como…** para guardar uma cópia, ou em **Substituir…** para escolher outro ficheiro. Para as imagens ligadas, existem também **Atualizar Ligação**, **Alterar Ligação…** e **Incorporar Imagem**. Consulte [Gestor de Gráficos](graphics-manager).

## Consulte também

- [Gestor de Gráficos](graphics-manager)
- [Desenhar formas e linhas](shapes)
- [Mover, alinhar, agrupar e sobrepor objetos](arrange)
- [O separador Formato da Imagem](tab-picture-format)
