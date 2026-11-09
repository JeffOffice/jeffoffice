# Abrir, guardar, imprimir e exportar
<!-- keywords: página web, html, guardar como página web, página web filtrada, correio eletrónico, eml, marcadores, marcadores de pdf, shift+f5, última edição, onde parou, modelos incorporados, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, ficheiros antigos, modelo, dotx, recentes, afixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não guardado, falha, ecrã ficheiro, formatos de ficheiro, guardar uma cópia, só de leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ott, reparar, abrir e reparar, danificado, palavra-passe, cifrar, proteger com palavra-passe -->

Clique em **Ficheiro** no canto superior esquerdo para ver o ecrã Ficheiro. Tem páginas para Novo, Abrir, Imprimir e Exportar, e comandos para Guardar e Guardar Como. Prima **Esc** para regressar ao documento.

## Iniciar um novo documento

Prima **Ctrl+N**, ou clique em **Ficheiro > Novo** e depois em **Documento em Branco**. Um novo documento abre numa janela própria. Se a janela em que está estiver vazia e nunca tiver sido guardada, o novo documento ocupa o seu lugar.

## Abrir um documento

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**. Clique em **Procurar** para escolher um ficheiro. O JeffWrite abre estes tipos de ficheiro:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento feito a partir dele, pelo que o modelo se mantém tal como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que vêm com um ficheiro. O JeffWrite nunca os executa. Mantém-nos como estavam quando guarda como .docm ou .dotm. Se guardar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite informa-o.
- **.txt** é texto simples, apenas as palavras. Ao abri-lo, o JeffWrite lê-o de imediato se for ASCII ou Unicode simples. Se não for, o JeffWrite pergunta em que codificação o deve ler e mostra as primeiras linhas, para que veja se as letras estão corretas. A **codificação** é a forma como um ficheiro guarda as letras, como UTF-8 ou Windows 1252. Ao guardar como .txt, escolhe a codificação e os finais de linha, e o ficheiro guarda apenas as palavras: sem tipos de letra, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. As marcas de lista chegam como marcas escritas, e as imagens e as ligações são deixadas de fora. Nunca vai à Internet buscar nada e ignora os scripts. Uma página web nunca é substituída: **Guardar** pede um novo nome e guarda um ficheiro .docx. Para criar uma página web, veja **Guardar como página web** mais abaixo.
- **.doc** e **.dot** são os tipos mais antigos de documento e de modelo. O JeffWrite lê-os e guarda o que fizer com eles como um novo ficheiro .docx.
- **.odt**, **.ott** e **.fodt** são ficheiros OpenDocument. O JeffWrite lê os três. Guarda ficheiros .odt e guarda o que fizer com um ficheiro .ott ou .fodt como um novo ficheiro .odt. Veja [Abrir e guardar ficheiros OpenDocument](open-odt).
- **.rtf** é Formato de Texto Enriquecido. O JeffWrite lê-o e guarda-o. Veja [Abrir ficheiros RTF](open-rtf).

A página **Abrir** também lista os documentos que abriu ou guardou recentemente. Clique num para o abrir. Clique com o botão direito num para o afixar no topo da lista, ou para o retirar da lista.

Se um ficheiro tiver partes que o JeffWrite ainda não consegue editar, como o esquema de uma tabela ou um cabeçalho, essas partes são mantidas. Veja [Bem-vindo ao JeffWrite](start). Se parte de um ficheiro não puder ser lida, o JeffWrite informa-o do que deixou de fora. Nesse caso, não guarda por cima desse ficheiro: **Guardar** pede um novo nome, para que o original mantenha o que a cópia perderia.

## Guardar um documento

Prima **Ctrl+S** para guardar. Da primeira vez, o JeffWrite pede um nome. **Guardar Como** (F12) guarda uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para guardar um modelo, **Documento com Macros (*.docm)** para manter as macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Formato de Texto Enriquecido (*.rtf)** para um ficheiro que outros programas consigam ler. Um ficheiro proveniente de um tipo mais antigo é sempre guardado com um novo nome, para que o ficheiro antigo nunca seja substituído.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse ficheiro, e continua a trabalhar no documento que tinha aberto. O nome e as alterações por guardar mantêm-se como estavam. Isto é útil quando quer enviar uma versão a alguém e continuar a escrever.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffWrite copia primeiro o ficheiro antigo para outro com o nome "Cópia de segurança de" seguido do nome original, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava. Assim, pode voltar à última versão guardada.

## Guardar como modelo

Um modelo é um documento que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo fica em **Os Meus Modelos**, e o documento em que está a escrever não é alterado. No **Guardar Como**, também pode escolher **Modelo (*.dotx)**, ou **Modelo de Texto OpenDocument (*.ott)**, para guardar um ficheiro de modelo onde preferir.

Para iniciar um novo documento a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos estão listados em **Os Meus Modelos**. Clique num deles. Abre-se um novo documento com o texto e os estilos do modelo. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de Recentes. Cada forma pede um ficheiro, ou usa aquele em que clicou:

- **Abrir Só de Leitura** abre o ficheiro apenas para leitura. Pode ver, deslocar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffWrite pede um novo nome, pelo que o ficheiro que abriu não é alterado.
- **Abrir como Cópia** abre um novo documento com o texto do ficheiro. Não tem ficheiro próprio, pelo que **Guardar** pergunta onde o colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** destina-se a um ficheiro .docx danificado, por exemplo um que foi cortado ao ser copiado. Veja mais abaixo.

## Abrir e reparar

Por vezes, um ficheiro .docx fica danificado. Uma cópia parou a meio, ou um disco cometeu erros. Um ficheiro .docx é um conjunto de partes mais pequenas, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que parou a meio é mantida até ao ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffWrite recria-as.

O documento reparado abre como cópia, com um nome como "Carta reparada". Uma mensagem indica o que foi mantido até uma interrupção e o que foi deixado de fora. O ficheiro danificado não é alterado. Guarde a cópia reparada com um nome próprio e verifique-a antes de confiar nela.

## Recuperar trabalho não guardado

O JeffWrite guarda uma cópia de reserva do trabalho não guardado a cada poucos minutos, num ficheiro na sua própria pasta. Se o programa fechar inesperadamente, oferece essas cópias na próxima vez que o iniciar. Clique em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado** para as ver mais tarde. Pode escolher quais abrir, e guardar cada uma onde quiser.

## Começar a partir de um modelo

**Ficheiro > Novo** mostra o **Documento em Branco** e um conjunto de modelos incluídos no JeffWrite, agrupados por tipo: carta, relatório, resumo, manuscrito e boletim informativo. Cada imagem é a primeira página do documento que obtém. Um **modelo** é um documento já preparado, com os seus estilos, margens e texto de exemplo. Clique num deles e substitua o texto de exemplo pelo seu. Os modelos que guardou são listados em **Os Meus Modelos**.

## Retomar onde parou

O JeffWrite recorda onde estava o cursor quando guardou ou fechou um ficheiro. Da próxima vez que abrir o ficheiro, o cursor está nesse ponto. Prima **Shift+F5** para ir aos locais editados por último. Cada toque leva ao anterior: os últimos três locais e, depois, de volta ao primeiro.

## Guardar como .odt

Um ficheiro .odt não pode conter tudo o que um ficheiro .docx pode. Ao guardar como .odt, o JeffWrite primeiro indica o que se perderia, e permite-lhe guardar um ficheiro .docx em vez disso. Veja [Informações](info).

## Imprimir

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**. Escolha uma impressora, as cópias e as páginas, veja cada página na pré-visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. Escolha as páginas a incluir: todas, a página atual ou um intervalo como 1-3, 5 (também funcionam p2s3 e s3). Depois, escolha um nome. O PDF tem o mesmo aspeto em qualquer computador, porque os tipos de letra ficam incorporados nele. O título, o autor, o assunto e as palavras-chave do documento passam para o PDF. Cada título torna-se um **marcador**, uma entrada na lista que um leitor de PDF mostra ao lado das páginas. Clique num marcador para ir à respetiva página.

## Guardar como página web

Clique em **Ficheiro > Exportar** e escolha um de dois tipos de página web. Uma **página web** é um ficheiro que pode abrir em qualquer navegador.

- **Guardar como Página Web** cria um único ficheiro .html que contém as suas palavras, o respetivo aspeto e as suas imagens. Pode enviar esse ficheiro sozinho. O aspeto que mantém é o tipo de letra, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre parágrafos.
- **Guardar como Página Web Filtrada** cria um pequeno ficheiro .html apenas com a estrutura: títulos, negrito e itálico, listas, tabelas, ligações e imagens. Não tem tipos de letra, tamanhos ou cores, pelo que pode dar-lhe depois o seu próprio aspeto.

Ambos mantêm títulos, listas com marcas e numeradas, tabelas com células unidas, ligações e imagens. O texto oculto, as palavras eliminadas numa alteração registada, os comentários, os cabeçalhos, os rodapés e as notas são deixados de fora. Uma ligação que iniciaria um programa ou executaria um script é guardada apenas como texto.

## Proteger um PDF com palavra-passe

Na caixa **Exportar para PDF**, clique em **Cifrar com uma Palavra-passe…**. Escreva a palavra-passe duas vezes. Quem abrir o PDF terá de a escrever. A palavra-passe protege realmente o ficheiro: as páginas são cifradas com encriptação forte (AES-256), e ninguém as consegue ler sem ela.

Também pode marcar **Restringir impressão, cópia e edição**, indicar uma segunda palavra-passe e escolher o que é permitido. Essas limitações são apenas um pedido. A maioria dos leitores de PDF respeita-as, mas não impedem quem queira contorná-las, por isso não confie nelas para manter um ficheiro privado.

Uma palavra-passe esquecida não pode ser recuperada, por isso guarde-a num lugar seguro. A palavra-passe é usada apenas para esse PDF e nunca é guardada. A caixa desativa o **PDF/A** enquanto houver uma palavra-passe definida, porque um ficheiro PDF/A não pode tê-la.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para conservação a longo prazo**. Um ficheiro PDF/A inclui todos os tipos de letra que usa e segue uma norma criada para conservar ficheiros durante muitos anos. Ainda não tem etiquetas para leitores de ecrã, e não pode ter palavra-passe.

## Criar um correio eletrónico

Clique em **Ficheiro > Exportar > Criar Correio Eletrónico** para criar um ficheiro de mensagem (.eml). Nada é enviado. Escolha o que vai na mensagem: o documento, o documento em PDF, ou as páginas apresentadas como imagens na mensagem (as primeiras 30 páginas). Abra o ficheiro no seu programa de correio eletrónico, indique o destinatário e envie-o. Para um PDF, pode clicar em **Cifrar com uma Palavra-passe…** na caixa; comunique a palavra-passe à pessoa por outro meio que não a própria mensagem.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Utilizar o JeffWrite com o teclado](keyboard)
