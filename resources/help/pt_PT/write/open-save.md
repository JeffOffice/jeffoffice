# Abrir, guardar, imprimir e exportar
<!-- keywords: tipos de letra no ficheiro, tipos de letra incorporados, tipo de letra em falta, livro eletrónico, e-book, epub, leitor de livros eletrónicos, capítulos, índice, página web, html, guardar como página web, página web filtrada, correio eletrónico, eml, marcadores, marcadores pdf, shift+f5, última edição, onde parou, modelos incluídos, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, ficheiros antigos, modelo, dotx, recentes, afixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não guardado, falha, ecrã ficheiro, formatos de ficheiro, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ott, reparar, abrir e reparar, danificado, palavra-passe, encriptar, proteger com palavra-passe -->

Clique em **Ficheiro**, no canto superior esquerdo, para ver o ecrã Ficheiro. Tem páginas para Novo, Abrir, Imprimir e Exportar, e comandos para Guardar e Guardar Como. Prima **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Prima **Ctrl+N**, ou clique em **Ficheiro > Novo** e, depois, em **Documento em Branco**. Um novo documento abre na sua própria janela. Se a janela em que está estiver vazia e nunca tiver sido guardada, o novo documento ocupa o seu lugar.

## Abrir um documento

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**. Clique em **Procurar** para escolher um ficheiro. O JeffWrite abre estes tipos de ficheiro:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento feito a partir dele, pelo que o modelo se mantém como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um ficheiro. O JeffWrite nunca os executa, e mantém-nos como estavam quando guarda como .docm ou .dotm. Se guardar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite avisa-o.
- **.txt** é texto simples, apenas as palavras. Ao abri-lo, o JeffWrite lê-o de imediato se for ASCII ou Unicode simples. Se não for, o JeffWrite pergunta qual a codificação a usar e mostra as primeiras linhas, para que veja se as letras estão corretas. Uma **codificação** é a forma como um ficheiro guarda as letras, como UTF-8 ou Windows 1252. Ao guardar como .txt, escolhe a codificação e os finais de linha, e o ficheiro guarda apenas as palavras: sem tipos de letra, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. As marcas e os números das listas chegam como marcas escritas, e as imagens e as ligações são deixadas de fora. Nunca vai buscar nada à Internet, e ignora os scripts. Uma página web nunca é substituída: **Guardar** pede um novo nome e guarda um ficheiro .docx. Para criar uma página web, veja **Guardar como página web** mais abaixo.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite lê-os e guarda o que faz com eles como um novo ficheiro .docx.
- **.odt**, **.ott** e **.fodt** são ficheiros OpenDocument. O JeffWrite lê os três. Guarda ficheiros .odt, e guarda o que faz com um ficheiro .ott ou .fodt como um novo ficheiro .odt. Veja [Abrir e guardar ficheiros OpenDocument](open-odt).
- **.rtf** é Formato de Texto Enriquecido. O JeffWrite lê-o e guarda-o. Veja [Abrir ficheiros RTF](open-rtf).

A página **Abrir** também lista os documentos que abriu ou guardou recentemente. Clique num para o abrir. Clique com o botão direito num para o afixar no topo da lista, ou para o retirar da lista.

Se um ficheiro tiver partes que o JeffWrite ainda não consegue editar, como o esquema de uma tabela ou um cabeçalho, mantém-nas. Veja [Bem-vindo ao JeffWrite](start). Se uma parte do ficheiro não puder ser lida de todo, o JeffWrite diz-lhe o que deixou de fora. Nesse caso não guarda por cima desse ficheiro: Guardar pede um novo nome, para que o original mantenha o que a cópia perdeu.

## Guardar um documento

Prima **Ctrl+S** para guardar. Na primeira vez, o JeffWrite pede um nome. **Guardar Como** (F12) guarda uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para guardar um modelo, **Documento com Macros (*.docm)** para manter as macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Formato de Texto Enriquecido (*.rtf)** para um ficheiro que outros programas consigam ler. Um ficheiro que veio de um tipo mais antigo é sempre guardado com um novo nome, para que o ficheiro antigo nunca seja substituído.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse ficheiro, e continua a trabalhar no documento que tinha. O nome e as alterações por guardar mantêm-se como estavam. Isto é útil quando quer enviar uma versão a alguém e continuar a escrever.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffWrite copia primeiro o ficheiro antigo para outro chamado "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve guardar sem ela. A resposta é Não, a não ser que diga Sim, e o ficheiro mantém-se como estava. Assim, pode voltar à última versão guardada.

## Guardar como modelo

Um modelo é um documento que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo fica em **Os Meus Modelos**, e o documento que está a escrever não é alterado. Em **Guardar Como**, também pode escolher **Modelo (*.dotx)** ou **Modelo de Texto OpenDocument (*.ott)** para guardar um ficheiro de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos aparecem em **Os Meus Modelos**. Clique num deles. Abre-se um novo documento com o texto e os estilos do modelo. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes. Cada forma pede um ficheiro, ou usa o que clicou:

- **Abrir Só de Leitura** abre o ficheiro apenas para leitura. Pode ver, percorrer, procurar e copiar, mas não pode alterar nada. A barra de título diz "Só de Leitura". Se premir **Guardar**, o JeffWrite pede um novo nome, pelo que o ficheiro que abriu não é alterado.
- **Abrir como Cópia** abre um novo documento com o texto do ficheiro. Não tem ficheiro próprio, por isso **Guardar** pergunta onde o colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** serve para um ficheiro .docx ou .doc danificado, por exemplo um que ficou incompleto ao ser copiado. Veja mais abaixo.

## Abrir e reparar

Por vezes um ficheiro .docx ou .doc fica danificado. Uma cópia parou a meio, ou um disco cometeu erros. Um ficheiro .docx é um conjunto de partes mais pequenas, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que parou a meio é mantida até ao ponto onde parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas desapareceram, o JeffWrite recria-as.

Um ficheiro .doc antigo é um grande invólucro com alguns fluxos de dados dentro. Um fluxo contém o texto, e outro contém a tabela que indica onde está cada pedaço de texto e qual é o seu aspeto. Se o invólucro estiver incompleto ou as suas listas danificadas, **Abrir e Reparar** encontra esses fluxos pelo conteúdo e recupera o máximo possível. Quando as listas que indicam como os pedaços se encaixam estão danificadas, o programa liga cada pedaço ao anterior e mostra o que encontra. Esse texto pode pertencer a uma cópia mais antiga do documento ou a outra parte do ficheiro, por isso a mensagem indica o parágrafo onde começa e pede-lhe que o compare com uma cópia de segurança, se tiver uma. Se a suposição vier antes do texto, a mensagem diz que o documento inteiro foi recomposto, e se vier depois do texto, diz que foi a formatação. Se não for possível recuperar totalmente as imagens, a mensagem indica-o. Se a tabela se perder, o texto é mantido como parágrafos simples, sem estilos, listas, tabelas ou imagens. Um ficheiro .doc com palavra-passe não pode ser reparado.

O documento reparado abre como cópia, com um nome como "Carta reparada". Uma mensagem diz-lhe o que foi mantido até uma interrupção e o que foi deixado de fora. O ficheiro danificado não é alterado. Guarde a cópia reparada com um nome próprio e verifique-a antes de confiar nela.

## Recuperar trabalho não guardado

O JeffWrite guarda uma cópia de reserva do trabalho não guardado a cada poucos minutos, num ficheiro na sua própria pasta. Se o programa fechar inesperadamente, oferece essas cópias na próxima vez que o iniciar. Clique em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado** para as ver mais tarde. Pode escolher quais abrir, e pode guardar cada uma onde quiser.

## Começar a partir de um modelo

**Ficheiro > Novo** mostra o **Documento em Branco** e um conjunto de modelos incluídos no JeffWrite, agrupados por tipo: uma carta, um relatório, um resumo, um manuscrito e um boletim informativo. Cada imagem é a primeira página do documento que obtém. Um **modelo** é um documento já preparado, com os seus estilos, margens e texto de exemplo. Clique num e substitua o texto de exemplo pelo seu. Os modelos que guardou são listados em **Os Meus Modelos**.

## Retomar onde parou

O JeffWrite lembra-se de onde estava o cursor quando guardou ou fechou um ficheiro. Da próxima vez que o abrir, o cursor estará nesse ponto. Prima **Shift+F5** para ir aos sítios editados por último. Cada toque leva ao anterior: os últimos três sítios e, depois, de volta ao primeiro.

## Guardar como .odt

Um ficheiro .odt não consegue conter tudo o que um ficheiro .docx contém. Ao guardar como .odt, o JeffWrite primeiro lista o que se perderia, e deixa-o guardar um ficheiro .docx em alternativa. Veja [Informações](info).

## Imprimir

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**. Escolha uma impressora, o número de cópias e as páginas, veja cada página na pré-visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. O PDF inclui o título, o autor, o assunto e as palavras-chave do documento, em **Ficheiro > Informações** (com **Incluir propriedades do documento** ativado), a data em que foi criado e o JeffWrite como programa que o criou; o autor e um título criado a partir do nome do ficheiro ficam de fora quando as informações pessoais são removidas. Cada um destes campos é mantido inteiro até 4000 carateres. Marque **Abrir o PDF após guardar** para que o ficheiro seja aberto no programa de PDF do seu computador quando for guardado. Esta opção está desativada até a marcar. Se o computador não conseguir abrir o ficheiro, a barra de estado indica-o (o ficheiro fica guardado na mesma). Escolha as páginas a incluir: todas, a página atual, ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Depois escolha como é criado:

- **Otimizar para** tem **Padrão**, que mantém as imagens como estão, e **Tamanho mínimo**, que limita cada imagem a 150 pixels por polegada, para que o ficheiro seja mais pequeno. Use Tamanho mínimo para um PDF que vai publicar numa página web ou enviar por correio eletrónico.
- **Incluir propriedades do documento** coloca no PDF o título, o autor, o assunto e as palavras-chave do documento.
- **Criar marcadores a partir dos títulos** transforma cada título num **marcador**, uma entrada na lista que um leitor de PDF mostra ao lado das páginas. Clique num marcador para ir à sua página.
- **Etiquetas de estrutura do documento para acessibilidade** marca os títulos, parágrafos, listas, tabelas (com as respetivas linhas de cabeçalho) e imagens (com o respetivo texto alternativo), e indica o idioma, para que um leitor de ecrã leia o PDF pela ordem correta.

Por fim, escolha um nome. O PDF tem o mesmo aspeto em qualquer computador, porque os tipos de letra ficam incorporados. O PDF que **Imprimir** cria ao escolher um ficheiro PDF também tem os marcadores e as etiquetas. Quando coloca várias páginas numa só folha, cada marcador e cada ligação aponta para a folha que contém a sua página, e as páginas de uma folha são lidas pela ordem em que estão dispostas: da esquerda para a direita, e depois de cima para baixo.

## Guardar como página web

Clique em **Ficheiro > Exportar** e escolha um de dois tipos de página web. Uma **página web** é um ficheiro que pode abrir em qualquer navegador.

- **Guardar como Página Web** cria um único ficheiro .html que contém as suas palavras, o respetivo aspeto e as suas imagens. Pode enviar esse ficheiro sozinho. O aspeto que mantém é o tipo de letra, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre parágrafos.
- **Guardar como Página Web Filtrada** cria um pequeno ficheiro .html apenas com a estrutura: títulos, negrito e itálico, listas, tabelas, ligações e imagens. Não tem tipos de letra, tamanhos ou cores, para que possa dar-lhe o seu próprio aspeto mais tarde.

Ambos mantêm os títulos, as listas com marcas e numeradas, as tabelas com células unidas, as ligações e as imagens. O texto oculto, as palavras que uma alteração controlada eliminou, os comentários, os cabeçalhos, os rodapés e as notas são deixados de fora. Uma ligação que iniciaria um programa ou executaria um script é guardada como texto simples.

## Guardar como livro eletrónico

Um **livro eletrónico** é um livro que lê num leitor de livros eletrónicos, num telemóvel ou num tablet. O ficheiro é um **EPUB**. As palavras adaptam-se a qualquer ecrã, pelo que o leitor pode aumentar ou diminuir as letras.

1. Clique em **Ficheiro > Exportar > Guardar como Livro Eletrónico (EPUB)**.
2. Verifique o **Título** e o **Autor**. Começam com o título e o autor das propriedades do documento (**Ficheiro > Informações**). Pode alterá-los apenas para este livro.
3. Ative **Usar a primeira imagem como capa** se quiser que a primeira imagem do documento seja a capa. Sem esta opção, o livro não tem capa.
4. Clique em **OK** e escolha onde guardar o ficheiro.

Como o livro é criado:

- Cada **Título 1** inicia um novo capítulo. As palavras antes do primeiro Título 1 formam um capítulo próprio.
- A lista de **índice** mostra todos os Títulos 1, 2 e 3, e o leitor pode ir diretamente a cada um. Os títulos mais pequenos estão no livro, mas não no índice.
- As imagens e as tabelas ficam onde as colocou. O texto alternativo de uma imagem acompanha-a, para que um leitor de ecrã possa dizer o que ela mostra.
- As notas de rodapé e as notas de fim são apresentadas no fim do capítulo em que estão. Cada nota tem uma ligação de volta ao ponto do texto, e cada número no texto é uma ligação para a sua nota.

Um livro está sujeito a limites, para que um ficheiro danificado não gere um livro enorme: até 500 capítulos, 4000 linhas no índice, 1000 imagens (nenhuma com mais de 32 MB) e 20 000 notas. Tudo o que ultrapassar um limite fica no último capítulo, ou passa a texto simples, e as suas palavras nunca são perdidas: uma imagem além do limite passa a ser o seu texto alternativo, e uma nota além do limite perde a formatação. Uma imagem que o leitor não consegue mostrar (um metaficheiro, um mapa de bits ou um TIFF) é convertida em PNG. Exportar de novo o mesmo documento guardado cria o mesmo livro, para que o leitor mantenha o seu lugar.

## Proteger um PDF com palavra-passe

Na caixa **Exportar para PDF**, clique em **Encriptar com uma palavra-passe…**. Escreva a palavra-passe duas vezes. Quem abrir o PDF terá de a escrever. A palavra-passe protege realmente o ficheiro: as páginas são cifradas com encriptação forte (AES-256), e ninguém as consegue ler sem ela.

Também pode marcar **Restringir impressão, cópia e edição**, indicar uma segunda palavra-passe e escolher o que é permitido. Estas limitações são apenas um pedido. A maioria dos leitores de PDF respeita-as, mas não impedem quem queira contorná-las, por isso não conte com elas para manter um ficheiro privado.

Uma palavra-passe esquecida não pode ser recuperada, por isso guarde-a num local seguro. A palavra-passe é usada apenas para esse PDF e nunca é guardada. Enquanto houver uma palavra-passe definida, a caixa desativa o **PDF/A**, porque um ficheiro PDF/A não pode ter palavra-passe.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para conservação a longo prazo**. Um ficheiro PDF/A (PDF/A-1b) contém todos os tipos de letra que usa, indica as suas cores e segue uma norma criada para conservar ficheiros durante muitos anos. Pode ter etiquetas para leitores de ecrã, mas não pode ter palavra-passe.

## Criar um correio eletrónico

Clique em **Ficheiro > Exportar > Criar Correio Eletrónico** para criar um ficheiro de mensagem (.eml). Nada é enviado. Escolha o que vai na mensagem: o documento, o documento como PDF, ou as páginas apresentadas como imagens na mensagem (as primeiras 30 páginas). Abra o ficheiro no seu programa de correio eletrónico, indique o destinatário e envie-o. Para um PDF, pode clicar em **Encriptar com uma palavra-passe…** na caixa; comunique a palavra-passe à pessoa por outro meio que não a própria mensagem.

## Tipos de letra no ficheiro

Um ficheiro .docx pode conter os tipos de letra em que o seu texto está composto, para que o documento tenha o aspeto que o autor lhe deu, mesmo num computador que não tenha esses tipos de letra. O JeffWrite lê-os quando abre o ficheiro. São usados apenas nesse documento, enquanto estiver aberto. Nunca são instalados no seu computador, nenhum outro documento ou programa os vê, e desaparecem quando fecha o documento. Um tipo de letra que o ficheiro contém não conta como tipo de letra em falta.

O JeffWrite não usa um tipo de letra cuja licença do criador não permita a incorporação, um tipo de letra danificado, nem nenhum tipo de letra para além dos primeiros 64 de um ficheiro. Quando isso acontece, a mensagem que vê ao abrir o ficheiro indica-o, e o texto é apresentado num tipo de letra semelhante. O ficheiro mantém os seus tipos de letra como estavam quando o guarda.

Para pôr nos ficheiros que guarda os tipos de letra usados no seu texto, clique em **Ficheiro > Opções**, abra a página **Guardar** e ative **Guardar tipos de letra no ficheiro**. Esta opção está desativada inicialmente. O JeffWrite inclui então os tipos de letra que vêm com ele e que o texto usa, em cada estilo usado no documento, ao guardar um ficheiro .docx. Nunca inclui um tipo de letra cuja licença não o permita, e quando deixa um de fora, a mensagem após a gravação indica o seu nome. O ficheiro fica maior, e no máximo 64 tipos de letra e 256 MB são incluídos num ficheiro.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Utilizar o JeffWrite com o teclado](keyboard)
