# Abrir, guardar, imprimir e exportar
<!-- keywords: livro eletrónico, ebook, epub, leitor de livros, capítulos, índice, página web, html, guardar como página web, página web filtrada, correio eletrónico, eml, marcadores, marcadores de PDF, shift+f5, última edição, onde parou, modelos incorporados, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, ficheiros antigos, modelo, dotx, recentes, afixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não guardado, falha, ecrã de ficheiro, formatos de ficheiro, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ott, reparar, abrir e reparar, danificado, palavra-passe, encriptar, proteger com palavra-passe -->

Clique em **Ficheiro**, no canto superior esquerdo, para ver o ecrã de Ficheiro. Tem páginas para Novo, Abrir, Imprimir e Exportar, e comandos para Guardar e Guardar como. Prima **Esc** para voltar ao seu documento.

## Iniciar um novo documento

Prima **Ctrl+N**, ou clique em **Ficheiro > Novo** e depois em **Documento em Branco**. Um novo documento abre na sua própria janela. Se a janela em que está estiver vazia e nunca tiver sido guardada, o novo documento ocupa o seu lugar.

## Abrir um documento

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**. Clique em **Procurar** para escolher um ficheiro. O JeffWrite abre estes tipos de ficheiro:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento feito a partir dele, pelo que o modelo se mantém como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um ficheiro. O JeffWrite nunca os executa, e mantém-nos como estavam quando guarda como .docm ou .dotm. Se guardar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite informa-o.
- **.txt** é texto simples, apenas as palavras. Ao abri-lo, o JeffWrite lê-o de imediato se for ASCII simples ou Unicode. Se não for, o JeffWrite pergunta em que **codificação** o deve ler e mostra as primeiras linhas, para que possa ver se as letras estão corretas. Uma **codificação** é a forma como um ficheiro guarda as letras, como UTF-8 ou Windows 1252. Ao guardar como .txt, escolhe a codificação e o fim das linhas, e o ficheiro guarda apenas as palavras: sem tipos de letra, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. As marcas das listas com marcas e números entram como marcas escritas, e as imagens e as ligações são deixadas de fora. Nunca transfere nada da Internet, e ignora os scripts. Uma página web nunca é substituída: **Guardar** pede um novo nome e guarda um ficheiro .docx. Para criar uma página web, veja **Guardar como página web** abaixo.
- **.doc** e **.dot** são os tipos mais antigos de documento e modelo. O JeffWrite lê-os, e guarda o que faz com eles como um novo ficheiro .docx.
- **.odt**, **.ott** e **.fodt** são ficheiros OpenDocument. O JeffWrite lê os três. Guarda ficheiros .odt, e guarda o que faz com um ficheiro .ott ou .fodt como um novo ficheiro .odt. Veja [Abrir e guardar ficheiros OpenDocument](open-odt).
- **.rtf** é Formato de Texto Enriquecido. O JeffWrite lê-o e guarda-o. Veja [Abrir ficheiros RTF](open-rtf).

A página **Abrir** também lista os documentos que abriu ou guardou recentemente. Clique num para o abrir. Clique com o botão direito num para o afixar no topo da lista, ou para o retirar da lista.

Se um ficheiro tiver partes que o JeffWrite ainda não consegue editar, como o esquema de uma tabela ou um cabeçalho, essas partes são mantidas. Veja [Bem-vindo ao JeffWrite](start). Se uma parte do ficheiro não puder ser lida de todo, o JeffWrite diz-lhe o que deixou de fora. Nesse caso, não guarda por cima desse ficheiro: **Guardar** pede um novo nome, para que o original mantenha o que a cópia perdeu.

## Guardar um documento

Prima **Ctrl+S** para guardar. Da primeira vez, o JeffWrite pede um nome. **Guardar como** (F12) guarda uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para guardar um modelo, **Documento com Macros (*.docm)** para manter as macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Formato de Texto Enriquecido (*.rtf)** para um ficheiro que outros programas possam ler. Um ficheiro que veio de um tipo de ficheiro mais antigo é sempre guardado com um novo nome, para que o ficheiro antigo nunca seja substituído.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse ficheiro, e continua a trabalhar no documento que tinha. O nome e as alterações por guardar mantêm-se como estavam. Isto é útil quando quer enviar uma versão a alguém e continuar a escrever.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guarda, o JeffWrite copia primeiro o ficheiro antigo para outro com o nome "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava. Assim, pode voltar à última versão guardada.

## Guardar como modelo

Um modelo é um documento que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo fica em **Os Meus Modelos**, e o documento que está a escrever não é alterado. Em **Guardar como**, também pode escolher **Modelo (*.dotx)**, ou **Modelo de Texto OpenDocument (*.ott)**, para guardar um ficheiro de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos aparecem listados em **Os Meus Modelos**. Clique num deles. Um novo documento abre com o texto e os estilos do modelo. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes. Cada forma pede um ficheiro, ou usa o que clicou:

- **Abrir Só de Leitura** abre o ficheiro apenas para leitura. Pode ver, deslocar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffWrite pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre um novo documento com o texto do ficheiro. Não tem ficheiro próprio, por isso **Guardar** pergunta onde o colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** destina-se a um ficheiro .docx ou .doc danificado, por exemplo um que foi cortado ao ser copiado. Veja abaixo.

## Abrir e Reparar

Por vezes, um ficheiro .docx ou .doc fica danificado. Uma cópia pode ter parado a meio, ou um disco pode ter cometido erros. Um ficheiro .docx é um conjunto de partes mais pequenas, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que parou a meio é mantida até ao ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffWrite volta a criá-las.

Um ficheiro .doc antigo é um grande invólucro com alguns fluxos de dados dentro. Um fluxo guarda o texto, e outro guarda a tabela que indica onde está cada pedaço de texto e qual é o seu aspeto. Se o invólucro tiver sido cortado ou as suas listas estiverem danificadas, **Abrir e Reparar** encontra esses fluxos pelo conteúdo que têm e lê o máximo que consegue. Se a tabela se tiver perdido, o texto é mantido como parágrafos simples, sem estilos, listas, tabelas ou imagens. Um ficheiro .doc que tenha palavra-passe não pode ser reparado.

O documento reparado abre como uma cópia com um nome como "Carta reparada". Uma mensagem indica o que foi mantido até uma interrupção e o que foi deixado de fora. O ficheiro danificado não é alterado. Guarde a cópia reparada com um nome próprio e verifique-a antes de confiar nela.

## Recuperar trabalho não guardado

O JeffWrite guarda de tempos a tempos uma cópia de reserva do trabalho não guardado, a cada poucos minutos, num ficheiro na sua própria pasta. Se o programa fechar inesperadamente, oferece essas cópias na próxima vez que o iniciar. Clique em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado** para as ver mais tarde. Pode escolher quais abrir, e pode guardar cada uma onde quiser.

## Iniciar a partir de um modelo

**Ficheiro > Novo** mostra o **Documento em Branco** e um conjunto de modelos incluídos no JeffWrite, agrupados por tipo: uma carta, um relatório, um resumo, um manuscrito e um boletim informativo. Cada imagem é a primeira página do documento que obtém. Um **modelo** é um documento já preparado, com os seus estilos, margens e texto de exemplo. Clique num, e substitua o texto de exemplo pelo seu. Os modelos que guardou você mesmo aparecem listados em **Os Meus Modelos**.

## Continuar onde parou

O JeffWrite lembra-se de onde estava o cursor quando guardou ou fechou um ficheiro. Na próxima vez que o abrir, o cursor está lá. Prima **Shift+F5** para ir aos locais que editou por último. Cada toque leva ao anterior: os últimos três locais, e depois volta ao primeiro.

## Guardar como .odt

Um ficheiro .odt não consegue guardar tudo o que um ficheiro .docx consegue. Ao guardar como .odt, o JeffWrite lista primeiro o que se perderia, e permite-lhe guardar um ficheiro .docx em vez disso. Veja [Informações](info).

## Imprimir

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**. Escolha uma impressora, o número de cópias e as páginas, veja cada página na pré-visualização, e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. Escolha as páginas a incluir: todas, a página atual, ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Depois escolha a forma como é criado:

- **Otimizar para** tem **Padrão**, que mantém as imagens como estão, e **Tamanho mínimo**, que faz com que cada imagem não tenha mais de 150 píxeis por polegada, para que o ficheiro seja mais pequeno. Use Tamanho mínimo para um PDF que vai colocar numa página web ou enviar por correio eletrónico.
- **Incluir propriedades do documento** coloca no PDF o título, o autor, o assunto e as palavras-chave do documento.
- **Criar marcadores a partir dos títulos** transforma cada título num **marcador**, uma entrada da lista que um leitor de PDF mostra ao lado das páginas. Clique num marcador para ir à respetiva página.
- **Etiquetas de estrutura do documento para acessibilidade** etiqueta os títulos, parágrafos, listas, tabelas (com as respetivas linhas de cabeçalho) e imagens (com o respetivo texto alternativo), e indica o idioma, para que um leitor de ecrã leia o PDF pela ordem correta.

Depois escolha um nome. O PDF tem o mesmo aspeto em qualquer computador, porque os tipos de letra ficam incluídos nele. O PDF que **Imprimir** cria quando escolhe um ficheiro PDF também tem os marcadores e as etiquetas. Quando coloca várias páginas numa folha, cada marcador e cada ligação aponta para a folha que contém a sua página, e as páginas de uma folha são lidas pela ordem em que estão nela: da esquerda para a direita, e de cima para baixo.

## Guardar como página web

Clique em **Ficheiro > Exportar** e escolha um dos dois tipos de página web. Uma **página web** é um ficheiro que pode abrir em qualquer navegador.

- **Guardar como Página Web** cria um único ficheiro .html que contém as suas palavras, o respetivo aspeto e as suas imagens. Pode enviar esse ficheiro sozinho. O aspeto mantido é o tipo de letra, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre parágrafos.
- **Guardar como Página Web Filtrada** cria um pequeno ficheiro .html apenas com a estrutura: títulos, negrito e itálico, listas, tabelas, ligações e imagens. Não tem tipos de letra, tamanhos nem cores, por isso pode dar-lhe depois o seu próprio aspeto.

Ambos mantêm os títulos, as listas com marcas e numeradas, as tabelas com células unidas, as ligações e as imagens. O texto oculto, as palavras que uma alteração registada eliminou, os comentários, os cabeçalhos, os rodapés e as notas são deixados de fora. Uma ligação que iniciaria um programa ou executaria um script é guardada como texto simples.

## Guardar como livro eletrónico

Um **livro eletrónico** é um livro que lê num leitor de livros, num telemóvel ou num tablet. O ficheiro é um **EPUB**. As palavras ajustam-se a qualquer ecrã, e o leitor pode aumentar ou diminuir as letras.

1. Clique em **Ficheiro > Exportar > Guardar como Livro Eletrónico (EPUB)**.
2. Verifique o **Título** e o **Autor**. Começam com o título e o autor das propriedades do documento (**Ficheiro > Informações**). Pode alterá-los apenas para este livro.
3. Ative **Usar a primeira imagem como capa** se quiser que a primeira imagem do documento seja a capa. Sem isto, o livro não tem capa.
4. Clique em **OK** e depois escolha onde guardar o ficheiro.

Assim se cria o livro:

- Cada **Título 1** inicia um novo capítulo. As palavras antes do primeiro Título 1 formam um capítulo próprio.
- O **índice** mostra todos os Título 1, Título 2 e Título 3, e o leitor pode ir diretamente a cada um. Os títulos mais pequenos estão no livro, mas não no índice.
- As imagens e as tabelas ficam onde as colocou. O texto alternativo de uma imagem acompanha-a, para que um leitor de ecrã possa dizer o que ela mostra.
- As notas de rodapé e as notas de fim são apresentadas no fim do capítulo em que estão. Cada nota tem uma ligação de volta ao ponto do texto, e cada número no texto é uma ligação para a sua nota.

Um livro está sujeito a limites, para que um ficheiro danificado não crie um livro enorme: até 500 capítulos, 4000 linhas no índice, 1000 imagens (nenhuma com mais de 32 MB) e 20 000 notas. O que ultrapassar um limite fica no último capítulo, ou torna-se texto simples, e as suas palavras nunca são eliminadas: uma imagem além do limite fica como o seu texto alternativo, e uma nota além do limite fica sem formatação. Uma imagem que um leitor não consegue mostrar (um metaficheiro, um mapa de bits ou um TIFF) é convertida em PNG. Exportar novamente o mesmo documento guardado cria o mesmo livro, para que um leitor mantenha o seu lugar.

## Proteger um PDF com palavra-passe

Na caixa **Exportar para PDF**, clique em **Encriptar com uma palavra-passe…**. Escreva a palavra-passe duas vezes. Quem abrir o PDF terá de a escrever. A palavra-passe protege realmente o ficheiro: as páginas são codificadas com encriptação forte (AES-256), e ninguém as consegue ler sem ela.

Também pode marcar **Restringir impressão, cópia e edição**, indicar uma segunda palavra-passe e escolher o que é permitido. Estas limitações são apenas um pedido. A maioria dos leitores de PDF respeita-as, mas não impedem quem queira contorná-las, por isso não conte com elas para manter um ficheiro privado.

Uma palavra-passe esquecida não pode ser recuperada, por isso guarde-a num local seguro. A palavra-passe é usada apenas para esse PDF e nunca é guardada. Enquanto houver uma palavra-passe definida, a caixa desativa o **PDF/A**, porque um ficheiro PDF/A não pode ter palavra-passe.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para conservação a longo prazo**. Um ficheiro PDF/A (PDF/A-1b) contém todos os tipos de letra que utiliza, indica as suas cores e segue uma norma criada para conservar ficheiros durante muitos anos. Pode ter etiquetas para leitores de ecrã, mas não pode ter palavra-passe.

## Criar um correio eletrónico

Clique em **Ficheiro > Exportar > Criar Correio Eletrónico** para criar um ficheiro de mensagem (.eml). Nada é enviado. Escolhe o que vai na mensagem: o documento, o documento como PDF, ou as páginas apresentadas como imagens na mensagem (as primeiras 30 páginas). Abra o ficheiro no seu programa de correio eletrónico, indique a quem se destina e envie-o. Para um PDF, pode clicar em **Encriptar com uma palavra-passe…** na caixa; indique a palavra-passe à pessoa por outra via que não na mensagem.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Utilizar o JeffWrite com o teclado](keyboard)
