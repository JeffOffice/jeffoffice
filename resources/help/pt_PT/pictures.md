# Adicionar e ajustar imagens
<!-- keywords: efeitos de imagem, sombra, reflexo, brilho, limites suaves, bisel, rotação 3D, repor imagem e tamanho, fotografia, foto, imagem, inserir imagem, recortar, clip art, página PDF, jpg, png, svg, recolorir, texto alternativo, legenda, marcador de posição, gestor de gráficos, imagens online, ligar a ficheiro, imagem ligada, inserir e ligar, incorporar, imagem em falta, religar -->

Uma imagem fica dentro de uma moldura. Pode recortar a imagem e mover ou redimensionar a moldura.

## Inserir uma imagem a partir de um ficheiro

1. Clique em **Inserir > Ilustrações > Imagens**.
2. Na janela **Inserir Imagem**, escolha um ficheiro. O JeffPub abre ficheiros PNG, JPEG, GIF, BMP, TIFF, WebP, SVG, WMF, EMF, ICO e PDF.
3. Escolha **Inserir**, **Ligar ao Ficheiro** ou **Inserir e Ligar** na caixa **Inserir como**, que fica junto à lista de ficheiros e pode ser alcançada com a tecla Tab. **Inserir** fica selecionada sempre que a janela abre. Veja "Ligar uma imagem ao seu ficheiro", mais abaixo.
4. Clique em **Abrir**.

A imagem fica no meio da página. Também pode arrastar um ficheiro de imagem para a página, o que usa sempre **Inserir**.

O JeffPub lê ficheiros de imagem até 256 MB. Um ficheiro maior é recusado, o mesmo acontecendo com um ficheiro vazio ou algo que não seja um ficheiro normal no disco, como um dispositivo ou um canal. A mensagem indica que o JeffPub não consegue ler o ficheiro como imagem.

Se escolher várias imagens de uma vez, ficam alinhadas como miniaturas na área de rascunho, o espaço cinzento ao lado da página. Arraste para a página as que quiser.

Se um PDF tiver mais do que uma página, a janela **Inserir Página PDF** mostra cada página. Escolha uma e clique em **OK**.

## Ligar uma imagem ao seu ficheiro

**Inserir** coloca uma cópia da imagem dentro da publicação. Essa cópia segue a publicação por onde quer que ela vá, mas uma publicação com muitas fotografias grandes também fica grande. Uma ligação mantém as fotografias nos respetivos ficheiros.

- **Inserir** copia a imagem para dentro da publicação. É a escolha habitual.
- **Ligar ao Ficheiro** guarda apenas a localização do ficheiro e uma pequena pré-visualização, com no máximo 512 pixels no seu lado mais longo. A imagem na página, e numa impressão, num PDF ou num livro eletrónico, vem do ficheiro com a qualidade total. Se o ficheiro for movido ou eliminado, a página mostra a pequena pré-visualização.
- **Inserir e Ligar** faz as duas coisas. A publicação guarda a imagem completa e a ligação. Quando abre a publicação e o ficheiro foi alterado, o JeffPub lê-o novamente e substitui a sua cópia.

O JeffPub memoriza a localização completa do ficheiro, e também a sua posição em relação à publicação, como "a pasta Fotografias ao lado dela". Se mover a publicação e as suas imagens em conjunto, a segunda indicação prevalece, e as imagens voltam a ser encontradas. Mantenha as pastas como estavam.

O JeffPub abre por si próprio o ficheiro de uma imagem ligada apenas quando o ficheiro está na pasta da publicação ou numa pasta dentro dela. Uma imagem ligada a um ficheiro noutro local mostra a pré-visualização guardada, desfocada, e fica marcada como **Não atualizada**, até a selecionar e clicar em **Atualizar Ligação** no Gestor de Gráficos. Isto impede que uma publicação que lhe enviem alcance o seu computador. Para que as imagens sejam carregadas automaticamente, guarde-as na pasta da publicação.

O **Gestor de Gráficos** mostra se cada imagem está **Incorporada**, **Ligada**, **Em falta**, **Modificada** ou **Não atualizada**, e tem botões para atualizar uma ligação, apontá-la para outro ficheiro ou transformá-la numa cópia. Veja [Gestor de Gráficos](graphics-manager).

Quando guarda uma publicação como ficheiro `.pub`, cria um ficheiro Pack and Go ou a envia por correio eletrónico, o JeffPub coloca cada imagem ligada completa no ficheiro, para que não dependa de ficheiros que ficam para trás. Se um ficheiro ligado estiver em falta nesse momento, ou se a imagem estiver **Não atualizada**, é incluída a sua pequena pré-visualização. Um ficheiro `.pub` que abre nunca tem ligações: as suas imagens entram como cópias.

## Adicionar um marcador de posição de imagem

Um marcador de posição é uma moldura vazia que reserva um lugar para uma imagem. Clique em **Inserir > Ilustrações > Marcador de Posição de Imagem**, depois faça duplo clique na moldura e escolha um ficheiro. A imagem preenche a moldura, e as arestas que sobressaem ficam ocultas. Um marcador de posição vazio não é impresso.

## Encontrar imagens gratuitas online

Clique em **Inserir > Ilustrações > Imagens Online**. O painel **Imagens Online** pesquisa duas bibliotecas gratuitas de imagens com licença aberta: a Openverse, que abrange muitas coleções, e a Wikimedia Commons, a biblioteca de imagens da Wikipédia.

1. Escolha uma biblioteca, escreva o que procura, como "celeiro vermelho", e prima Enter.
2. Clique numa imagem para ver quem a criou e a respetiva licença. Uma licença é o conjunto de regras para utilizar a imagem. A maioria pede que indique o nome do autor.
3. Clique em **Inserir**. O JeffPub coloca a imagem na página e usa o seu título como texto alternativo. Com **Adicionar um crédito por baixo da imagem** marcado, por baixo dela aparece uma linha como "Celeiro vermelho" de Pat Lee, CC BY 2.0.

As palavras da pesquisa são enviadas para a biblioteca que escolheu. Verifique se a licença de cada imagem se adequa à forma como a vai utilizar.

Uma pesquisa mostra até 24 imagens. O JeffPub utiliza apenas endereços Web seguros (https) de uma biblioteca, e deixa de fora uma imagem cujo endereço não seja um desses. Na Wikimedia Commons, uma imagem com mais de 1920 pixels de largura é transferida com essa largura, e não no original carregado. Uma transferência superior a 50 MB, ou que demore demasiado, é interrompida, e o painel indica que a imagem não pôde ser transferida nem carregada.

## Recortar uma imagem

Recortar oculta as margens de uma imagem.

1. Clique na imagem e depois em **Formato da Imagem > Recortar > Recortar**.
2. Arraste uma alça para ocultar parte da imagem. Arraste a imagem para a deslocar dentro da moldura.
3. Clique novamente em **Recortar**, ou prima **Esc**.

Também no grupo **Recortar**: **Proporção** corta a moldura a uma forma como um quadrado, 4 de largura por 3 de altura, ou 16 por 9, em torno do centro (a imagem mantém-se onde está na página); **Ajustar** mostra a imagem inteira dentro da moldura; **Preencher** preenche a moldura; **Recortar para Forma** ajusta a imagem a uma forma como uma elipse; e **Limpar Recorte** volta a mostrar a imagem inteira.

## Alterar o aspeto de uma imagem

No separador **Formato da Imagem**, use o grupo **Ajustar**:

- **Remoção de Fundo**: retira o fundo de uma imagem, como a parede lisa atrás de uma pessoa ou o branco à volta de um logótipo. Clique na imagem e depois em **Remoção de Fundo**. O JeffPub encontra as cores ao longo das arestas da imagem e pinta a rosa o que parece ser fundo. Se retirou demasiado ou muito pouco, clique em **Marcar Áreas a Manter** ou **Marcar Áreas a Remover** no separador **Remoção de Fundo** e arraste sobre a imagem; onde duas marcas se encontram, prevalece a que desenhou por último. **Manter Alterações** (ou Enter) termina a operação e mantém o resultado, e **Descartar Todas as Alterações** (ou Esc) repõe tudo. A sessão inteira é um único passo de anulação. A imagem nunca é cortada: clique novamente em **Remoção de Fundo** para continuar a marcar, ou em **Repor Imagem** para trazer de volta a imagem completa. Uma imagem muito grande mostra uma caixa com **Cancelar** enquanto o JeffPub trabalha. Os desenhos feitos de linhas e formas (SVG) e as imagens animadas não podem ter o fundo removido. Um ficheiro .jpub guarda a remoção para que possa voltar a editá-la. **Repor Imagem** retira a remoção feita aqui; **Alterar Imagem** não está disponível enquanto marca. Ir para outra página, eliminar a imagem ou guardar termina a Remoção de Fundo e mantém o que marcou. Um ficheiro .pub, um PDF, uma página Web ou um livro eletrónico guardam apenas a imagem final, e a Verificação de Compatibilidade indica-o para os .pub. A imagem final é guardada com o tamanho em que é apresentada, por isso pode parecer suave se outro programa a ampliar, e o JeffPub reconstrói-a a partir do original sempre que guarda aqui.
- **Correções**: escolha um par de luminosidade e contraste, de menos 40 a mais 40 por cento, ou **Aguçar** ou **Suavizar** (25% e 50% em cada sentido). **Formatar Objeto…**, na parte inferior do menu, tem controlos deslizantes.
- **Recolorir** também tem cinco opções de **Temperatura**: valores negativos são mais frios e azulados, valores positivos são mais quentes e avermelhados.
- **Efeitos Artísticos**: mostra uma pequena amostra da imagem em cada efeito (**Desfocagem**, **Lápis em Escala de Cinzentos**, **Realçar Contornos** e **Mosaico**). Escolha um para o usar, ou **Sem Efeito Artístico** para o retirar. Cada escolha é um passo de anulação e **Repor Imagem** retira-o. A própria imagem nunca é alterada. Num ficheiro .pub guarda-se a imagem final, e a Verificação de Compatibilidade indica que o efeito não pode ser alterado aí. Um efeito não retira a imagem original do ficheiro: **Desfocagem** e **Mosaico** não ocultam nada a quem receba o ficheiro, porque o original continua nele. Para retirar o original, escolha **Ficheiro**, depois **Informações** e depois **Inspecionar Publicação**, e clique em **Remover Originais**, junto a **Imagens originais mantidas sob efeitos de imagem**. A imagem passa a manter apenas o aspeto final, e **Repor Imagem** já não consegue trazer o original de volta. A mesma lista tem **Partes recortadas de imagens**: uma imagem recortada continua a conter as partes que cortou e que não se veem, e **Remover Partes Recortadas** reduz cada imagem ao que é mostrado, para que a página fique exatamente igual (uma imagem com efeito é primeiro convertida na sua imagem final, e uma imagem desenhada mais pequena do que está guardada fica como está, o que a lista indica). Enquanto um efeito está a ser calculado, uma imagem com **Desfocagem** ou **Mosaico** aparece como uma mancha lisa da sua cor média, nunca como a imagem que oculta, e a apresentação de diapositivos, a impressão e a gravação aguardam até o cálculo terminar.
- **Recolorir**: torna a imagem em Escala de Cinzentos, Sépia, Desvanecido, Preto e Branco, ou numa tonalidade de destaque. **Definir Cor Transparente** torna transparente a cor do canto superior esquerdo da imagem.
- **Transparência…**: esbate a imagem inteira.
- **Repor Imagem**: retira todas as alterações desta lista, bem como a borda, a forma e os efeitos da imagem. O recorte também é retirado, pelo que a moldura mostra a imagem inteira. Clique na seta ao lado para **Repor Imagem e Tamanho**, que também devolve a imagem ao tamanho que tem no seu ficheiro.
- **Comprimir Imagens…**: reduz o ficheiro baixando o detalhe para 300, 220 ou 96 ppp (pixels por polegada). Por omissão, altera todas as imagens e elimina as áreas recortadas.
- **Converter em Formas**: transforma uma imagem SVG (um desenho feito de linhas e formas) em formas editáveis, recortadas onde o desenho as corta. O texto e as imagens que estejam dentro dela são deixados de fora.

## Estilos, limites e efeitos de imagem

Use o grupo **Estilos de Imagem** no separador **Formato da Imagem**:

- A **galeria** mostra cerca de duas dezenas de aspetos prontos: molduras, sombras, reflexos, limites suaves, formas arredondadas e ovais, uma rotação no espaço, um brilho e biséis. Um bisel é uma aresta inclinada que faz a imagem parecer elevada. Cada miniatura mostra as suas cores. Clique numa para dar esse aspeto à imagem. Um estilo define em conjunto a forma, o limite e os efeitos, e substitui os que a imagem tinha. O recorte, o tamanho e as cores mantêm-se.
- **Contorno da Imagem** define a cor, a **Espessura** (a largura da linha) e os **Tracejados** (uma linha contínua, pontilhada ou tracejada). **Sem Contorno** retira o limite.
- **Efeitos de Imagem** abre **Sombra**, **Reflexo**, **Brilho**, **Limites Suaves**, **Bisel** e **Rotação 3D**. Cada um tem opções prontas. Escolha uma para a adicionar. Os outros efeitos mantêm-se. Escolha a primeira entrada, como **Sem Sombra**, para retirar um. **Mais Opções de Efeitos…** abre **Formatar Objeto**.

Cada uma destas ações é um passo que pode anular.

## Legenda, texto alternativo e troca

- **Formato da Imagem > Estilos de Imagem > Legenda** adiciona uma caixa de texto por baixo da imagem.
- O texto alternativo é uma descrição breve para quem não consegue ver a imagem, como os utilizadores de leitores de ecrã. Clique com o botão direito na imagem, escolha **Formatar Objeto…**, abra o separador **Texto Alternativo** e escreva em **Texto alternativo**.
- **Formato da Imagem > Inserir > Alterar Imagem** coloca outro ficheiro na mesma moldura. A moldura mantém o seu tamanho, o seu lugar e o seu aspeto: o limite, a forma, os efeitos e as alterações de cor. Se a nova imagem tiver a mesma forma da antiga, o recorte também se mantém. Se tiver outra forma, preenche a moldura.
- Para trocar duas imagens, arraste uma sobre a outra. Quando a indicação disser **Solte para trocar as imagens**, largue: as imagens mudam de moldura, e a moldura que arrastou volta ao seu lugar. Ou mantenha Shift premido, clique nas duas e clique em **Trocar**. Mantenha Ctrl premido ao arrastar para copiar a imagem. Uma cópia que largue sobre outra imagem fica onde a colocou, e nada é trocado.

## Ver todas as imagens

Clique em **Ver > Mostrar > Gestor de Gráficos** para listar todas as imagens no painel **Gestor de Gráficos**. Selecione uma e clique em **Ir para** para lhe ir diretamente, em **Guardar Como…** para guardar uma cópia, ou em **Substituir…** para escolher outro ficheiro. Para as imagens ligadas, tem também **Atualizar Ligação**, **Alterar Ligação…** e **Incorporar Imagem**. Veja [Gestor de Gráficos](graphics-manager).

## Consulte também

- [Gestor de Gráficos](graphics-manager)
- [Desenhar formas e linhas](shapes)
- [Mover, alinhar, agrupar e sobrepor objetos](arrange)
- [O separador Formato da Imagem](tab-picture-format)
