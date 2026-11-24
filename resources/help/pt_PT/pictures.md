# Adicionar e ajustar imagens
<!-- keywords: efeitos de imagem, sombra, reflexo, brilho, limites suaves, bisel, rotação 3D, repor imagem e tamanho, foto, imagem, inserir imagem, recortar, imagem de arquivo, página PDF, jpg, png, svg, recolorir, texto alternativo, legenda, marcador de posição, gestor de gráficos, imagens online, ligar a ficheiro, imagem ligada, inserir e ligar, incorporar, imagem em falta, voltar a ligar -->

Uma imagem fica dentro de uma moldura. Pode recortar a imagem e mover ou redimensionar a moldura.

## Inserir uma imagem a partir de um ficheiro

1. Clique em **Inserir > Ilustrações > Imagens**.
2. Na janela **Inserir Imagem**, escolha um ficheiro. O JeffPub abre ficheiros PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Escolha **Inserir**, **Ligar ao Ficheiro** ou **Inserir e Ligar** na caixa **Inserir como**, que está junto à lista de ficheiros e pode ser alcançada com a tecla Tab. **Inserir** é a opção escolhida sempre que a janela se abre. Veja "Ligar uma imagem ao respetivo ficheiro", mais abaixo.
4. Clique em **Abrir**.

A imagem fica no meio da página. Também pode arrastar um ficheiro de imagem para a página, o que usa sempre **Inserir**.

O JeffPub lê ficheiros de imagem até 256 MB. Um ficheiro maior é recusado, o mesmo acontecendo com um ficheiro vazio ou com algo que não seja um ficheiro normal no disco, como um dispositivo ou um canal. A mensagem diz que o JeffPub não consegue ler o ficheiro como imagem.

Se escolher várias imagens de uma vez, elas ficam alinhadas como miniaturas na área de rascunho, o espaço cinzento ao lado da página. Arraste para a página as que quiser.

Se um PDF tiver mais do que uma página, a janela **Inserir Página PDF** mostra cada página. Escolha uma e clique em **OK**.

## Ligar uma imagem ao respetivo ficheiro

**Inserir** coloca uma cópia da imagem dentro da publicação. Essa cópia segue a publicação por onde quer que ela vá, mas uma publicação com muitas fotografias grandes também fica grande. Uma ligação mantém as fotografias nos respetivos ficheiros.

- **Inserir** copia a imagem para a publicação. É a escolha habitual.
- **Ligar ao Ficheiro** guarda apenas a localização do ficheiro e uma pequena pré-visualização, com no máximo 512 píxeis no lado mais comprido. A imagem na página, e numa impressão, num PDF ou num livro eletrónico, vem do ficheiro com a qualidade total. Se o ficheiro for movido ou eliminado, a página mostra a pequena pré-visualização.
- **Inserir e Ligar** faz as duas coisas. A publicação guarda a imagem completa e a ligação. Quando abre a publicação e o ficheiro foi alterado, o JeffPub lê-o de novo e substitui a sua cópia.

O JeffPub memoriza a localização completa do ficheiro e também a sua posição em relação à publicação, como "a pasta Fotografias ao lado dela". Se mover a publicação e as respetivas imagens em conjunto, a segunda indicação prevalece, pelo que as imagens voltam a ser encontradas. Mantenha as pastas como estavam.

O JeffPub só abre por si o ficheiro de uma imagem ligada quando o ficheiro está na pasta da publicação ou numa pasta dentro dela. Uma imagem ligada a um ficheiro noutro lugar mostra a pré-visualização guardada, desfocada, e fica marcada como **Não atualizada**, até a selecionar e clicar em **Atualizar Ligação** no Gestor de Gráficos. Assim, uma publicação que lhe enviem não consegue aceder ao seu computador. Para que as imagens carreguem automaticamente, guarde-as na pasta da publicação.

O **Gestor de Gráficos** mostra se cada imagem está **Incorporada**, **Ligada**, **Em falta**, **Modificada** ou **Não atualizada**, e tem botões para atualizar uma ligação, apontá-la para outro ficheiro ou transformá-la numa cópia. Veja [Gestor de Gráficos](graphics-manager).

Ao guardar uma publicação como ficheiro `.pub`, criar um ficheiro Pack and Go ou enviá-la por correio eletrónico, o JeffPub coloca cada imagem ligada completa dentro do ficheiro, para que não dependa de ficheiros que ficam para trás. Se um ficheiro ligado estiver em falta nesse momento, ou a imagem estiver **Não atualizada**, entra em seu lugar a respetiva pequena pré-visualização. Um ficheiro `.pub` que abra nunca tem ligações: as imagens entram como cópias.

## Adicionar um marcador de posição de imagem

Um marcador de posição é uma moldura vazia que reserva um lugar para uma imagem. Clique em **Inserir > Ilustrações > Marcador de Posição de Imagem**, depois faça duplo clique na moldura e escolha um ficheiro. A imagem preenche a moldura, e as arestas que sobressaem ficam ocultas. Um marcador de posição vazio não é impresso.

## Encontrar imagens gratuitas online

Clique em **Inserir > Ilustrações > Imagens Online**. O painel **Imagens Online** pesquisa duas bibliotecas gratuitas de imagens com licença aberta: o Openverse, que abrange muitas coleções, e o Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Escolha uma biblioteca, escreva o que procura, como "celeiro vermelho", e prima Enter.
2. Clique numa imagem para ver quem a criou e a respetiva licença. A licença é o conjunto de regras para usar a imagem. A maioria pede que indique o nome do autor.
3. Clique em **Inserir**. O JeffPub coloca a imagem na página e usa o respetivo título como texto alternativo. Com **Adicionar um crédito por baixo da imagem** marcado, fica por baixo uma linha como "Celeiro vermelho" de Pat Lee, CC BY 2.0.

As palavras da pesquisa são enviadas para a biblioteca que escolheu. Verifique se a licença de cada imagem se adequa à forma como a vai usar.

Uma pesquisa mostra até 24 imagens. O JeffPub usa apenas endereços Web seguros (https) de uma biblioteca e deixa de fora uma imagem cujo endereço não seja um desses. Do Wikimedia Commons, uma imagem com mais de 1920 píxeis de largura chega como cópia com essa largura, e não como o original carregado. Um descarregamento acima de 50 MB, ou um que demore demasiado tempo, é interrompido, e o painel indica que a imagem não pôde ser descarregada nem carregada.

## Recortar uma imagem

Recortar oculta as margens de uma imagem.

1. Clique na imagem e depois em **Formato da Imagem > Recortar > Recortar**.
2. Arraste uma alça para ocultar parte da imagem. Arraste a imagem para a deslocar dentro da moldura.
3. Clique novamente em **Recortar**, ou prima **Esc**.

Também no grupo **Recortar**: **Proporção** corta a moldura a uma forma como um quadrado, 4 de largura por 3 de altura, ou 16 por 9, em torno do centro (a imagem mantém-se onde está na página); **Ajustar** mostra a imagem inteira dentro da moldura; **Preencher** preenche a moldura; **Recortar para Forma** recorta para uma forma como uma elipse; e **Limpar Recorte** mostra novamente a imagem inteira.

## Alterar o aspeto de uma imagem

No separador **Formato da Imagem**, use o grupo **Ajustar**:

- **Remoção de Fundo**: retira o fundo de uma imagem, como a parede lisa atrás de uma pessoa ou o branco à volta de um logótipo. Clique na imagem e depois em **Remover Fundo**. O JeffPub encontra as cores ao longo das arestas da imagem e tinge de cor-de-rosa o que parece ser fundo. Se retirou demasiado ou de menos, clique em **Marcar Áreas a Manter** ou **Marcar Áreas a Remover** no separador **Remoção de Fundo** e arraste sobre a imagem; onde duas marcas se encontram, prevalece a que desenhou por último. **Manter Alterações** (ou Enter) termina e mantém o resultado, e **Descartar Todas as Alterações** (ou Esc) repõe tudo. A sessão inteira conta como um passo para anular. A imagem nunca é cortada: clique novamente em **Remover Fundo** para continuar a marcar, ou em **Repor Imagem** para trazer de volta a imagem inteira. Uma imagem muito grande mostra uma caixa com **Cancelar** enquanto o JeffPub trabalha. Desenhos feitos de linhas e formas (SVG) e imagens animadas não podem ter o fundo removido. Um ficheiro .jpub guarda a remoção para que a possa voltar a editar. **Repor Imagem** anula a remoção feita aqui; **Alterar Imagem** não está disponível enquanto marca. Ir para outra página, eliminar a imagem ou guardar termina a Remoção de Fundo e mantém o que marcou. Um ficheiro .pub, um PDF, uma página Web ou um livro eletrónico guardam apenas a imagem final, e a Verificação de Compatibilidade indica-o para o .pub. A imagem final é guardada com o tamanho em que é apresentada, por isso pode ficar suave se outro programa a ampliar, e o JeffPub reconstrói-a a partir do original sempre que guarda aqui.
- **Correções**: escolha um par de luminosidade e contraste, de menos 40 a mais 40 por cento. **Formatar Objeto…**, no fim do menu, tem controlos deslizantes.
- **Recolorir**: torna a imagem em Escala de Cinzentos, Sépia, Desvanecido, Preto e Branco, ou com uma tonalidade de destaque. **Definir Cor Transparente** torna transparente a cor do canto superior esquerdo da imagem.
- **Transparência…**: esbate a imagem inteira.
- **Repor Imagem**: anula todas as alterações desta lista, bem como o limite, a forma e os efeitos da imagem. O recorte também é anulado, pelo que a moldura mostra a imagem inteira. Clique na seta ao lado para **Repor Imagem e Tamanho**, que também devolve a imagem ao tamanho que tem no respetivo ficheiro.
- **Comprimir Imagens…**: reduz o ficheiro baixando o detalhe para 300, 220 ou 96 ppi (píxeis por polegada). Por predefinição, altera todas as imagens e elimina as áreas recortadas.
- **Converter em Formas**: transforma uma imagem SVG (um desenho feito de linhas e formas) em formas que pode editar, recortadas onde o desenho as corta. O texto e as imagens que existam dentro dela ficam de fora.

## Estilos, limites e efeitos de imagem

Use o grupo **Estilos de Imagem** no separador **Formato da Imagem**:

- A **galeria** mostra cerca de duas dezenas de aspetos prontos: molduras, sombras, reflexos, limites suaves, formas arredondadas e ovais, uma rotação no espaço, um brilho e biséis. Um bisel é uma aresta inclinada que faz a imagem parecer elevada. Cada miniatura mostra as suas cores. Clique numa para dar à imagem esse aspeto. Um estilo define a forma, o limite e os efeitos em conjunto, e substitui os que a imagem tinha. O recorte, o tamanho e as cores mantêm-se.
- **Limite da Imagem** define a cor, a **Espessura** (a grossura da linha) e os **Tracejados** (uma linha contínua, pontilhada ou tracejada). **Sem Contorno** retira o limite.
- **Efeitos de Imagem** abre **Sombra**, **Reflexo**, **Brilho**, **Limites Suaves**, **Bisel** e **Rotação 3D**. Cada um tem opções prontas. Escolha uma para a adicionar. Os outros efeitos mantêm-se. Escolha a primeira entrada, como **Sem Sombra**, para retirar um. **Mais Opções de Efeitos…** abre **Formatar Objeto**.

Cada uma destas ações é um passo que pode anular.

## Legenda, texto alternativo e troca

- **Formato da Imagem > Estilos de Imagem > Legenda** adiciona uma caixa de texto por baixo da imagem.
- O texto alternativo é uma descrição curta para pessoas que não conseguem ver a imagem, como os utilizadores de leitores de ecrã. Clique com o botão direito na imagem, escolha **Formatar Objeto…**, abra o separador **Texto Alternativo** e escreva em **Texto alternativo**.
- **Formato da Imagem > Inserir > Alterar Imagem** coloca outro ficheiro na mesma moldura. A moldura mantém o tamanho, o lugar e o aspeto: o limite, a forma, os efeitos e as alterações de cor. Se a nova imagem tiver a mesma forma que a antiga, o recorte também se mantém. Se tiver outra forma, preenche a moldura.
- Para trocar duas imagens, arraste uma para cima da outra. Quando a dica disser **Solte para trocar as imagens**, largue: as imagens mudam de moldura, e a moldura que arrastou volta ao lugar onde estava. Ou mantenha Shift premido, clique nas duas e clique em **Trocar**. Mantenha Ctrl premido ao arrastar para copiar a imagem. Uma cópia largada sobre outra imagem fica onde a colocou, e nada é trocado.

## Ver todas as imagens

Clique em **Ver > Mostrar > Gestor de Gráficos** para listar todas as imagens no painel **Gestor de Gráficos**. Selecione uma e clique em **Ir para** para ir até ela, em **Guardar Como…** para guardar uma cópia, ou em **Substituir…** para escolher outro ficheiro. Para imagens ligadas, tem também **Atualizar Ligação**, **Alterar Ligação…** e **Incorporar Imagem**. Veja [Gestor de Gráficos](graphics-manager).

## Relacionados

- [Gestor de Gráficos](graphics-manager)
- [Desenhar formas e linhas](shapes)
- [Mover, alinhar, agrupar e sobrepor objetos](arrange)
- [O separador Formato da Imagem](tab-picture-format)
