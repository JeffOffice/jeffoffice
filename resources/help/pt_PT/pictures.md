# Adicionar e ajustar imagens
<!-- keywords: fotografia, foto, imagem, inserir imagem, recortar, cortar, clip art, página PDF, jpg, png, svg, recolorir, texto alternativo, legenda, marcador de posição, gestor de gráficos, imagens online, ligar ao ficheiro, imagem ligada, inserir e ligar, incorporar, imagem em falta, voltar a ligar -->

Uma imagem fica dentro de uma moldura. Pode recortar a imagem e mover ou redimensionar a moldura.

## Inserir uma imagem a partir de um ficheiro

1. Clique em **Inserir > Ilustrações > Imagens**.
2. Na janela **Inserir Imagem**, escolha um ficheiro. O JeffPub abre ficheiros PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Escolha **Inserir**, **Ligar ao Ficheiro** ou **Inserir e Ligar** na caixa **Inserir como**, que se encontra junto à lista de ficheiros e pode ser alcançada com a tecla Tab. A opção **Inserir** fica selecionada sempre que a janela abre. Consulte "Ligar uma imagem ao respetivo ficheiro", mais abaixo.
4. Clique em **Abrir**.

A imagem é colocada no meio da página. Também pode arrastar um ficheiro de imagem para a página, o que utiliza sempre **Inserir**.

O JeffPub lê ficheiros de imagem até 256 MB. Um ficheiro maior é recusado, tal como um ficheiro vazio ou algo que não seja um ficheiro normal no disco, como um dispositivo ou um pipe. A mensagem indica que o JeffPub não o consegue ler como imagem.

Se escolher várias imagens de uma vez, estas ficam alinhadas como miniaturas na área de rascunho, o espaço cinzento ao lado da página. Arraste as que pretende para a página.

Se um PDF tiver mais de uma página, a janela **Inserir Página PDF** mostra cada página. Escolha uma e clique em **OK**.

## Ligar uma imagem ao respetivo ficheiro

**Inserir** coloca uma cópia da imagem dentro da publicação. Essa cópia acompanha a publicação para onde quer que vá, mas uma publicação com muitas fotografias grandes também fica grande. Uma ligação mantém as fotografias nos respetivos ficheiros.

- **Inserir** copia a imagem para a publicação. É a opção habitual.
- **Ligar ao Ficheiro** guarda apenas a localização do ficheiro e uma pequena pré-visualização, com no máximo 512 píxeis no lado maior. A imagem na página, bem como numa impressão, num PDF ou num livro eletrónico, provém do ficheiro com qualidade total. Se o ficheiro for movido ou eliminado, a página mostra a pequena pré-visualização.
- **Inserir e Ligar** faz as duas coisas. A publicação guarda a imagem completa e a ligação. Quando abre a publicação e o ficheiro foi alterado, o JeffPub volta a lê-lo e substitui a sua cópia.

O JeffPub memoriza a localização completa do ficheiro e também a sua posição relativamente à publicação, como "a pasta Fotografias ao lado dela". Se mover a publicação e as imagens em conjunto, prevalece a segunda, pelo que as imagens voltam a ser encontradas. Mantenha as pastas como estavam.

O JeffPub só abre automaticamente o ficheiro de uma imagem ligada quando este se encontra na pasta da publicação ou numa pasta dentro dela. Uma imagem ligada a um ficheiro noutro local mostra a pré-visualização guardada, desfocada, e fica marcada como **Não atualizada**, até a selecionar e clicar em **Atualizar Ligação** no Gestor de Gráficos. Isto impede que uma publicação que alguém lhe envie aceda ao seu computador. Para que as imagens sejam carregadas automaticamente, mantenha-as na pasta da publicação.

O **Gestor de Gráficos** mostra se cada imagem está **Incorporada**, **Ligada**, **Em falta**, **Modificado** ou **Não atualizada**, e tem botões para atualizar uma ligação, apontá-la para outro ficheiro ou transformá-la numa cópia. Consulte [Gestor de Gráficos](graphics-manager).

Quando guarda uma publicação como ficheiro `.pub`, cria um ficheiro Pack and Go ou a envia por correio eletrónico, o JeffPub coloca cada imagem ligada no ficheiro na íntegra, para que não dependa de ficheiros que ficam para trás. Se, nesse momento, um ficheiro ligado estiver em falta ou a imagem estiver **Não atualizada**, é incluída a pequena pré-visualização. Um ficheiro `.pub` que abra nunca tem ligações: as imagens são importadas como cópias.

## Adicionar um marcador de posição de imagem

Um marcador de posição é uma moldura vazia que reserva espaço para uma imagem. Clique em **Inserir > Ilustrações > Marcador de posição de imagem**, faça duplo clique na moldura e escolha um ficheiro. A imagem preenche a moldura e as margens que ficam de fora são ocultadas. Um marcador de posição vazio não é impresso.

## Encontrar imagens gratuitas online

Clique em **Inserir > Ilustrações > Imagens online**. O painel **Imagens online** pesquisa duas bibliotecas gratuitas de imagens com licenças abertas: o Openverse, que abrange muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Escolha uma biblioteca, escreva o que procura, como "celeiro vermelho", e prima Enter.
2. Clique numa imagem para ver quem a criou e qual a respetiva licença. Uma licença é o conjunto de regras de utilização da imagem. A maioria pede que indique o autor.
3. Clique em **Inserir**. O JeffPub coloca a imagem na página e utiliza o título como texto alternativo. Com **Adicionar um crédito por baixo da imagem** selecionado, é colocada por baixo uma linha como "Celeiro vermelho" por Pat Lee, CC BY 2.0.

As palavras pesquisadas são enviadas para a biblioteca escolhida. Verifique se a licença de cada imagem é adequada à utilização que lhe vai dar.

Uma pesquisa mostra até 24 imagens. O JeffPub utiliza apenas endereços Web seguros (https) de uma biblioteca e exclui as imagens cujo endereço não o seja. Do Wikimedia Commons, uma imagem com mais de 1920 píxeis de largura é obtida como cópia com essa largura, e não como o original carregado. Uma transferência com mais de 50 MB, ou que demore demasiado tempo, é interrompida, e o painel indica que não foi possível transferir ou carregar a imagem.

## Recortar uma imagem

O recorte oculta as margens de uma imagem.

1. Clique na imagem e, em seguida, clique em **Formato da Imagem > Recortar > Recortar**.
2. Arraste uma alça para ocultar parte da imagem. Arraste a imagem para a deslizar dentro da moldura.
3. Clique novamente em **Recortar** ou prima **Esc**.

Também no grupo **Recortar**: **Ajustar** mostra a imagem completa na moldura, **Preenchimento** preenche a moldura, **Recortar para Forma** recorta segundo uma forma, como uma oval, e **Limpar Recorte** volta a mostrar a imagem completa.

## Alterar o aspeto de uma imagem

No separador **Formato da Imagem**, utilize o grupo **Ajustar**:

- **Correções**: escolha uma combinação de luminosidade e contraste, de menos 40 a mais 40 por cento. **Formatar Objeto…**, na parte inferior do menu, tem controlos de deslize.
- **Recolorir**: torne a imagem Escala de cinzentos, Sépia, Desvanecido, Preto e Branco ou uma tonalidade de destaque. **Definir Cor Transparente** torna transparente a cor do canto superior esquerdo da imagem.
- **Transparência…**: esbate a imagem inteira.
- **Repor Imagem**: anula estas alterações.
- **Comprimir Imagens…**: reduz o ficheiro diminuindo o detalhe para 300, 220 ou 96 ppi (píxeis por polegada). Por predefinição, altera todas as imagens e elimina as áreas recortadas.
- **Converter em Formas**: transforma uma imagem SVG (um desenho feito de linhas e formas) em formas editáveis, cortadas onde o desenho as recorta. O texto e as imagens incluídos são excluídos.

## Legenda, texto alternativo e troca

- **Formato da Imagem > Estilos de Imagem > Legenda** adiciona uma caixa de texto por baixo da imagem.
- O texto alternativo é uma breve descrição para pessoas que não conseguem ver a imagem, como os utilizadores de leitores de ecrã. Clique com o botão direito do rato na imagem, escolha **Formatar Objeto…**, abra o separador **Texto Alternativo** e escreva em **Texto alternativo**.
- **Formato da Imagem > Inserir > Alterar Imagem** coloca outro ficheiro na mesma moldura.
- Para trocar duas imagens, arraste uma para cima da outra. Quando a sugestão indicar **Solte para trocar as imagens**, solte: as imagens trocam de moldura e a moldura que arrastou volta ao seu lugar. Em alternativa, mantenha premida a tecla Shift, clique em ambas e clique em **Trocar**. Mantenha premida a tecla Ctrl enquanto arrasta para copiar uma imagem. Uma cópia que solte sobre outra imagem fica onde a colocou e não há troca.

## Ver todas as imagens

Clique em **Ver > Mostrar > Gestor de Gráficos** para listar todas as imagens no painel **Gestor de Gráficos**. Selecione uma e clique em **Ir para** para saltar para ela, em **Guardar Como…** para guardar uma cópia ou em **Substituir…** para escolher outro ficheiro. Para imagens ligadas, tem também **Atualizar Ligação**, **Alterar Ligação…** e **Incorporar Imagem**. Consulte [Gestor de Gráficos](graphics-manager).

## Tópicos relacionados

- [Gestor de Gráficos](graphics-manager)
- [Desenhar formas e linhas](shapes)
- [Mover, alinhar, agrupar e dispor objetos em camadas](arrange)
- [O separador Formato da Imagem](tab-picture-format)
